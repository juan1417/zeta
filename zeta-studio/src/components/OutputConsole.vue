<template>
  <v-card flat tile style="height: 100%; display: flex; flex-direction: column">
    <div ref="consoleRef" class="console-output">
      <div
        v-for="(line, i) in output"
        :key="i"
        :class="['console-line', line.startsWith('[error]') ? 'error' : 'output']"
      >
        {{ line }}
      </div>
      <div v-if="output.length === 0" class="console-line muted">
        Output will appear here...
      </div>
    </div>
    <v-divider />
    <div class="d-flex align-center pa-1" style="flex-shrink: 0">
      <v-text-field
        v-model="commandInput"
        density="compact"
        variant="outlined"
        hide-details
        placeholder="Evaluate expression..."
        class="mr-2"
        @keydown.enter="evalExpression"
        bg-color="surface"
      />
      <v-btn
        icon="mdi-send"
        size="small"
        variant="tonal"
        color="primary"
        @click="evalExpression"
      />
    </div>
  </v-card>
</template>

<script setup lang="ts">
import { ref, watch, nextTick } from 'vue'
import { useZetaStore } from '../stores/zeta'
import { storeToRefs } from 'pinia'

const store = useZetaStore()
const { output } = storeToRefs(store)

const consoleRef = ref<HTMLElement | null>(null)
const commandInput = ref('')

watch(
  output,
  async () => {
    await nextTick()
    if (consoleRef.value) {
      consoleRef.value.scrollTop = consoleRef.value.scrollHeight
    }
  },
  { deep: true }
)

async function evalExpression() {
  const expr = commandInput.value.trim()
  if (!expr) return
  commandInput.value = ''
  store.output.push(`> ${expr}`)
  try {
    await store.exec(expr)
  } catch {
    // error already pushed by store.exec
  }
}
</script>

<style scoped>
.console-output {
  flex: 1;
  overflow-y: auto;
  padding: 8px 12px;
  font-family: 'Cascadia Code', 'Fira Code', monospace;
  font-size: 13px;
  line-height: 1.5;
  background: #1e1e1e;
}
.console-line.output {
  color: #e0e0e0;
}
.console-line.error {
  color: #f44336;
}
.console-line.muted {
  color: #666;
  font-style: italic;
}
</style>
