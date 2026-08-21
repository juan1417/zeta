<template>
  <v-toolbar density="compact" color="surface">
    <v-btn
      :icon="isRunning ? 'mdi-stop' : 'mdi-play'"
      :color="isRunning ? 'error' : 'success'"
      size="small"
      @click="toggleExecution"
      :disabled="isRunning && !canStop"
    />
    <v-btn icon="mdi-delete" size="small" @click="clearOutput" title="Clear Output" />
    <v-divider vertical class="mx-2" />
    <v-chip size="small" :color="isRunning ? 'warning' : 'success'" variant="tonal">
      <v-icon start :icon="isRunning ? 'mdi-loading' : 'mdi-check-circle'" />
      {{ isRunning ? 'Running...' : 'Ready' }}
    </v-chip>
    <v-spacer />
    <span v-if="isRunning" class="text-caption text-medium-emphasis">
      {{ elapsedTime }}s
    </span>
    <span class="text-caption text-medium-emphasis">
      Zeta v0.2.0
    </span>
  </v-toolbar>
</template>

<script setup lang="ts">
import { ref, onUnmounted } from 'vue'
import { useZetaStore } from '../stores/zeta'

const store = useZetaStore()
const isRunning = ref(false)
const canStop = ref(false)
const elapsedTime = ref(0)
let timer: ReturnType<typeof setInterval> | null = null

async function toggleExecution() {
  if (isRunning.value) {
    stopExecution()
  } else {
    await startExecution()
  }
}

async function startExecution() {
  isRunning.value = true
  elapsedTime.value = 0
  timer = setInterval(() => { elapsedTime.value++ }, 1000)

  const zeta = (window as any).__zeta
  const code = zeta?.getCode?.() || ''

  if (!code.trim()) {
    store.output.push('[info] No code to execute')
    isRunning.value = false
    if (timer) { clearInterval(timer); timer = null }
    return
  }

  try {
    await store.exec(code)
  } finally {
    isRunning.value = false
    if (timer) { clearInterval(timer); timer = null }
  }
}

function stopExecution() {
  isRunning.value = false
  if (timer) {
    clearInterval(timer)
    timer = null
  }
}

function clearOutput() {
  store.clearOutput()
}

onUnmounted(() => {
  if (timer) {
    clearInterval(timer)
  }
})
</script>
