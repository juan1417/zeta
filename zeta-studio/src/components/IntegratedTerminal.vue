<template>
  <v-card flat tile style="height: 100%">
    <v-toolbar density="compact" color="primary">
      <v-toolbar-title class="text-subtitle-1">Terminal</v-toolbar-title>
      <v-spacer />
      <v-btn icon="mdi-plus" size="small" @click="createTerminal" title="New Terminal" />
      <v-btn icon="mdi-delete" size="small" @click="clearTerminal" title="Clear" />
    </v-toolbar>
    <div ref="terminalContainer" style="height: calc(100% - 40px); overflow: hidden"></div>
  </v-card>
</template>

<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { Terminal } from 'xterm'
import { FitAddon } from '@xterm/addon-fit'
import 'xterm/css/xterm.css'

const terminalContainer = ref<HTMLElement>()
let terminal: Terminal | null = null
let fitAddon: FitAddon | null = null
let resizeObserver: ResizeObserver | null = null

onMounted(() => {
  initTerminal()
  if (terminalContainer.value) {
    resizeObserver = new ResizeObserver(() => {
      fitAddon?.fit()
    })
    resizeObserver.observe(terminalContainer.value)
  }
})

onUnmounted(() => {
  resizeObserver?.disconnect()
  terminal?.dispose()
})

function initTerminal() {
  if (!terminalContainer.value) return

  terminal = new Terminal({
    theme: {
      background: '#1e1e2e',
      foreground: '#cdd6f4',
      cursor: '#f5e0dc',
      cursorAccent: '#1e1e2e',
      selectionBackground: '#585b7066',
      black: '#45475a',
      red: '#f38ba8',
      green: '#a6e3a1',
      yellow: '#f9e2af',
      blue: '#89b4fa',
      magenta: '#f5c2e7',
      cyan: '#94e2d5',
      white: '#bac2de',
    },
    fontFamily: '"Fira Code", "Cascadia Code", monospace',
    fontSize: 14,
    cursorBlink: true,
  })

  fitAddon = new FitAddon()
  terminal.loadAddon(fitAddon)
  terminal.open(terminalContainer.value)
  fitAddon.fit()

  terminal.onData((data) => {
    window.__TAURI__?.invoke('terminal_input', { data })
  })

  window.__TAURI__?.listen('terminal-data', (event) => {
    terminal?.write(event.payload as string)
  })

  terminal.writeln('\x1b[1;36mZeta Studio Terminal\x1b[0m')
  terminal.writeln('\x1b[90mType commands or run Zeta scripts.\x1b[0m')
  terminal.writeln('')
}

function createTerminal() {
  terminal?.clear()
  window.__TAURI__?.invoke('create_terminal')
}

function clearTerminal() {
  terminal?.clear()
}
</script>
