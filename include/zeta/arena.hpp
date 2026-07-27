#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>
#include <memory>
#include <functional>
#include <type_traits>
#include <new>
#include <cassert>

namespace zeta {

// Arena allocator — region-based memory.
// Allocate with create(), bulk-free with reset().
// Individual destroy() is NOT allowed (by design — Rust model).
// This is intentional: you cannot free individual objects from an arena.
// Objects live until the arena is reset or destroyed.
//
// Usage:
//   Arena arena;
//   int* p = arena.create<int>(42);      // allocated in arena
//   double* d = arena.create<double>(3.14);
//   arena.reset();  // ALL objects freed at once
//
class Arena {
public:
    // Block sizes start at 64KB, double up to 1MB
    static constexpr std::size_t MIN_BLOCK_SIZE = 64 * 1024;
    static constexpr std::size_t MAX_BLOCK_SIZE = 1024 * 1024;

    Arena() = default;

    // Non-copyable, movable
    Arena(const Arena&) = delete;
    Arena& operator=(const Arena&) = delete;

    Arena(Arena&& other) noexcept
        : blocks_(std::move(other.blocks_))
        , total_allocated_(other.total_allocated_)
        , current_block_(other.current_block_)
        , current_offset_(other.current_offset_) {
        other.blocks_.clear();
        other.total_allocated_ = 0;
        other.current_block_ = 0;
        other.current_offset_ = 0;
    }

    Arena& operator=(Arena&& other) noexcept {
        if (this != &other) {
            blocks_ = std::move(other.blocks_);
            total_allocated_ = other.total_allocated_;
            current_block_ = other.current_block_;
            current_offset_ = other.current_offset_;
            other.blocks_.clear();
            other.total_allocated_ = 0;
            other.current_block_ = 0;
            other.current_offset_ = 0;
        }
        return *this;
    }

    ~Arena() = default;

    // Allocate a T in the arena. T must be trivially destructible OR
    // you must call destructors manually before reset().
    //
    // For types with non-trivial destructors (std::string, std::vector, etc.),
    // we wrap them in a TrackedDtor that calls the destructor at arena reset.
    template<typename T, typename... Args>
    T* create(Args&&... args) {
        // For trivially destructible types: raw placement new
        if constexpr (std::is_trivially_destructible_v<T>) {
            return allocate_raw<T>(std::forward<Args>(args)...);
        } else {
            // For types with non-trivial destructors: tracked destruction
            return allocate_tracked<T>(std::forward<Args>(args)...);
        }
    }

    // Reset all memory — O(1) amortized
    // Calls destructors for tracked objects
    void reset() {
        // Call destructors in reverse order
        for (auto it = tracked_dtors_.rbegin(); it != tracked_dtors_.rend(); ++it) {
            (*it)();
        }
        tracked_dtors_.clear();

        // Reset blocks
        for (auto& block : blocks_) {
            block.used = 0;
        }
        current_block_ = 0;
        current_offset_ = 0;
        total_allocated_ = 0;
    }

    // Stats
    std::size_t bytes_used() const { return total_allocated_; }
    std::size_t block_count() const { return blocks_.size(); }

    // Alignment helpers
    static std::size_t align_up(std::size_t value, std::size_t alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

private:
    struct Block {
        std::vector<std::uint8_t> data;
        std::size_t used = 0;

        explicit Block(std::size_t size) : data(size) {}
    };

    std::vector<Block> blocks_;
    std::size_t total_allocated_ = 0;
    std::size_t current_block_ = 0;
    std::size_t current_offset_ = 0;

    // Destructors for non-trivially-destructible objects
    std::vector<std::function<void()>> tracked_dtors_;

    // Allocate raw memory for trivially destructible types
    template<typename T, typename... Args>
    T* allocate_raw(Args&&... args) {
        constexpr std::size_t size = sizeof(T);
        constexpr std::size_t align = alignof(T);

        ensure_space(size, align);

        Block& block = blocks_[current_block_];
        std::size_t aligned_offset = align_up(current_offset_, align);
        void* mem = block.data.data() + aligned_offset;

        current_offset_ = aligned_offset + size;
        total_allocated_ += size;

        return ::new (mem) T(std::forward<Args>(args)...);
    }

    // Allocate with destructor tracking
    template<typename T, typename... Args>
    T* allocate_tracked(Args&&... args) {
        constexpr std::size_t size = sizeof(T);
        constexpr std::size_t align = alignof(T);

        ensure_space(size, align);

        Block& block = blocks_[current_block_];
        std::size_t aligned_offset = align_up(current_offset_, align);
        void* mem = block.data.data() + aligned_offset;

        current_offset_ = aligned_offset + size;
        total_allocated_ += size;

        T* obj = ::new (mem) T(std::forward<Args>(args)...);

        // Track destructor
        tracked_dtors_.push_back([obj]() {
            obj->~T();
        });

        return obj;
    }

    void ensure_space(std::size_t size, std::size_t align) {
        // Check current block
        if (current_block_ < blocks_.size()) {
            Block& block = blocks_[current_block_];
            std::size_t aligned_offset = align_up(current_offset_, align);
            if (aligned_offset + size <= block.data.size()) {
                return; // Fits in current block
            }
        }

        // Need new block
        std::size_t block_size = std::max(
            MIN_BLOCK_SIZE,
            std::min(MAX_BLOCK_SIZE, align_up(size + align, MIN_BLOCK_SIZE))
        );

        blocks_.emplace_back(block_size);
        current_block_ = blocks_.size() - 1;
        current_offset_ = 0;
    }
};

// Convenience: arena-allocated vector of values
// (for when you need a collection of arena-allocated objects)
template<typename T>
class ArenaVec {
public:
    ArenaVec(Arena& arena, std::size_t initial_capacity = 16)
        : arena_(arena) {
        data_ = arena_.create<T[]>(initial_capacity);
        capacity_ = initial_capacity;
    }

    void push(T value) {
        if (size_ >= capacity_) {
            grow();
        }
        data_[size_++] = std::move(value);
    }

    T& operator[](std::size_t idx) { return data_[idx]; }
    const T& operator[](std::size_t idx) const { return data_[idx]; }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    T* begin() { return data_; }
    T* end() { return data_ + size_; }

private:
    Arena& arena_;
    T* data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

    void grow() {
        std::size_t new_cap = capacity_ * 2;
        T* new_data = arena_.create<T[]>(new_cap);
        std::memcpy(new_data, data_, sizeof(T) * size_);
        data_ = new_data;
        capacity_ = new_cap;
    }
};

} // namespace zeta
