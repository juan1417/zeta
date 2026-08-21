<template>
  <v-card flat tile style="height: 100%; display: flex; flex-direction: column">
    <div v-if="plotDataUrl" class="plot-container">
      <img :src="plotDataUrl" alt="Plot" class="plot-image" />
    </div>
    <div v-else class="plot-placeholder">
      <v-icon size="64" color="grey-darken-1">mdi-chart-line</v-icon>
      <div class="text-grey mt-2">No plot yet</div>
      <div class="text-grey text-caption">Run a script with render/display to see plots</div>
    </div>
    <v-divider />
    <div class="d-flex pa-1" style="flex-shrink: 0">
      <v-btn
        size="small"
        variant="tonal"
        color="primary"
        @click="refreshPlot"
        :loading="loading"
        prepend-icon="mdi-refresh"
      >
        Refresh
      </v-btn>
      <v-spacer />
      <v-btn
        v-if="plotDataUrl"
        size="small"
        variant="tonal"
        color="secondary"
        @click="downloadPlot"
        prepend-icon="mdi-download"
      >
        Save PNG
      </v-btn>
    </div>
  </v-card>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { invoke } from '@tauri-apps/api/core'
import { save } from '@tauri-apps/plugin-dialog'
import { writeFile } from '@tauri-apps/plugin-fs'
import { useZetaStore } from '../stores/zeta'

const store = useZetaStore()

const plotDataUrl = ref<string | null>(null)
const loading = ref(false)

async function refreshPlot() {
  loading.value = true
  try {
    const pngBytes = await invoke<number[]>('get_plot')
    if (pngBytes && pngBytes.length > 0) {
      const blob = new Blob([new Uint8Array(pngBytes)], { type: 'image/png' })
      plotDataUrl.value = URL.createObjectURL(blob)
    } else {
      plotDataUrl.value = null
    }
  } catch {
    plotDataUrl.value = null
    store.output.push('[info] No plot available')
  } finally {
    loading.value = false
  }
}

async function downloadPlot() {
  if (!plotDataUrl.value) return
  try {
    const path = await save({
      defaultPath: 'plot.png',
      filters: [{ name: 'PNG', extensions: ['png'] }],
    })
    if (path) {
      const pngBytes = await invoke<number[]>('get_plot')
      await writeFile(path, new Uint8Array(pngBytes))
      store.output.push(`[info] Plot saved to ${path}`)
    }
  } catch (e) {
    store.output.push(`[error] Save failed: ${e}`)
  }
}
</script>

<style scoped>
.plot-container {
  flex: 1;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 8px;
  overflow: hidden;
}
.plot-image {
  max-width: 100%;
  max-height: 100%;
  object-fit: contain;
}
.plot-placeholder {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
}
</style>
