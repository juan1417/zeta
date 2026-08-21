# Zeta Studio

A desktop IDE for the Zeta data analysis language, built with Tauri.

## Tech Stack

- **Backend**: Rust + Tauri 2
- **Frontend**: Vue 3 + Vuetify 3 + TypeScript
- **Build**: Vite

## Prerequisites

- Rust + Cargo
- Node.js + npm
- `zeta` binary in PATH (or configure the path in the app)

## Development

```bash
npm install
npm run tauri dev
```

## Build

```bash
npm run tauri build
```

The binary will be in `src-tauri/target/release/`.
