<template>
  <v-app theme="dark">
    <v-main>
      <v-container fluid class="pa-0" style="height: 100vh">
        <v-row no-gutters style="height: 100%">
          <v-col cols="2" style="height: 100%">
            <FileExplorer @open-file="handleOpenFile" />
          </v-col>
          <v-col cols="7" style="height: 100%">
            <v-row no-gutters style="height: 100%">
              <v-col cols="12">
                <RunBar />
              </v-col>
              <v-col cols="12" style="height: calc(100% - 48px)">
                <EditorPanel />
              </v-col>
            </v-row>
          </v-col>
          <v-col cols="3" style="height: 100%">
            <v-card flat tile style="height: 100%">
              <v-tabs v-model="rightTab" density="compact" color="primary">
                <v-tab value="variables">Variables</v-tab>
                <v-tab value="output">Output</v-tab>
                <v-tab value="plots">Plots</v-tab>
              </v-tabs>
              <v-window v-model="rightTab" style="height: calc(100% - 40px)">
                <v-window-item value="variables" style="height: 100%">
                  <VariableExplorer />
                </v-window-item>
                <v-window-item value="output" style="height: 100%">
                  <OutputConsole />
                </v-window-item>
                <v-window-item value="plots" style="height: 100%">
                  <PlotViewer />
                </v-window-item>
              </v-window>
            </v-card>
          </v-col>
        </v-row>
      </v-container>
    </v-main>

    <v-footer app density="compact" class="text-caption text-medium-emphasis">
      <span>Zeta {{ zetaVersion }}</span>
      <v-spacer />
      <span v-if="currentFile">{{ currentFile }}</span>
      <span v-else>No file open</span>
    </v-footer>
  </v-app>
</template>

<script setup lang="ts">
import { ref, onMounted } from 'vue'
import { invoke } from '@tauri-apps/api/core'
import FileExplorer from './components/FileExplorer.vue'
import RunBar from './components/RunBar.vue'
import EditorPanel from './components/EditorPanel.vue'
import VariableExplorer from './components/VariableExplorer.vue'
import OutputConsole from './components/OutputConsole.vue'
import PlotViewer from './components/PlotViewer.vue'
import { useZetaStore } from './stores/zeta'
import { storeToRefs } from 'pinia'

const store = useZetaStore()
const { currentFile } = storeToRefs(store)

const rightTab = ref('output')
const zetaVersion = ref('')

onMounted(async () => {
  try {
    zetaVersion.value = await invoke('get_version')
  } catch {
    zetaVersion.value = 'dev'
  }
})

async function handleOpenFile(path: string) {
  try {
    const content = await invoke<string>('read_file', { path })
    store.currentFile = path
    store.currentCode = content
  } catch (e) {
    store.output.push(`[error] Failed to open file: ${e}`)
  }
}
</script>

<style>
html, body, #app {
  margin: 0;
  padding: 0;
  height: 100%;
  overflow: hidden;
}
</style>
