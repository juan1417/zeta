<template>
  <v-card flat tile style="height: 100%">
    <v-tabs v-model="activeTab" density="compact" color="primary" show-arrows>
      <v-tab v-for="(file, i) in openFiles" :key="file.path" :value="i">
        <span class="text-body-2">{{ file.name }}</span>
        <v-icon size="small" class="ml-1" @click.stop="closeTab(i)">mdi-close</v-icon>
      </v-tab>
    </v-tabs>
    <div style="position: relative; height: calc(100% - 40px)">
      <div ref="editorContainer" style="width: 100%; height: 100%"></div>
      <div
        v-if="openFiles.length === 0"
        class="d-flex align-center justify-center"
        style="position: absolute; inset: 0; background: #1e1e2e"
      >
        <div class="text-center text-medium-emphasis">
          <v-icon size="48" class="mb-2">mdi-file-document-outline</v-icon>
          <div class="text-body-1">No files open</div>
          <div class="text-caption">Open or create a file from the explorer</div>
        </div>
      </div>
    </div>
  </v-card>
</template>

<script setup lang="ts">
import { ref, watch, onMounted, onBeforeUnmount, nextTick } from 'vue'
import editor from '@monaco-editor/loader'
import type { editor as MonacoEditor, IDisposable } from 'monaco-editor'
import { useZetaStore } from '../stores/zeta'
import { storeToRefs } from 'pinia'
import { zetaLanguage, zetaMonarch } from '../zeta-language'

const store = useZetaStore()
const { currentFile, currentCode } = storeToRefs(store)

const editorContainer = ref<HTMLElement | null>(null)
const activeTab = ref(0)
const openFiles = ref<{ name: string; path: string; content: string; model: MonacoEditor.ITextModel | null }[]>([])

let monacoInstance: MonacoEditor.IStandaloneCodeEditor | null = null
let monacoModule: typeof import('monaco-editor') | null = null
let modelChangeListener: IDisposable | null = null

function onZetaRun() {
  if (currentCode.value.trim()) {
    store.exec(currentCode.value)
  }
}

function onZetaSave() {
  store.saveFile()
}

onMounted(async () => {
  window.addEventListener('zeta-run', onZetaRun)
  window.addEventListener('zeta-save', onZetaSave)

  monacoModule = await editor.init()
  if (!monacoModule || !editorContainer.value) return

  monacoModule.languages.register({ id: 'zeta' })
  monacoModule.languages.setLanguageConfiguration('zeta', zetaLanguage)
  monacoModule.languages.setMonarchTokensProvider('zeta', zetaMonarch)

  monacoInstance = monacoModule.editor.create(editorContainer.value, {
    value: '',
    language: 'zeta',
    theme: 'vs-dark',
    minimap: { enabled: false },
    fontSize: 14,
    automaticLayout: true,
    scrollBeyondLastLine: false,
    padding: { top: 8 },
  })

  if (monacoInstance) {
    monacoInstance.addCommand(monacoModule.KeyMod.CtrlCmd | monacoModule.KeyCode.Enter, () => {
      window.dispatchEvent(new CustomEvent('zeta-run'))
    })

    monacoInstance.addCommand(monacoModule.KeyMod.CtrlCmd | monacoModule.KeyCode.KeyS, () => {
      window.dispatchEvent(new CustomEvent('zeta-save'))
    })
  }
})

onBeforeUnmount(() => {
  window.removeEventListener('zeta-run', onZetaRun)
  window.removeEventListener('zeta-save', onZetaSave)
  if (modelChangeListener) {
    modelChangeListener.dispose()
  }
  openFiles.value.forEach(f => f.model?.dispose())
  monacoInstance?.dispose()
})

function attachModel(index: number) {
  if (!monacoInstance || !monacoModule) return
  const file = openFiles.value[index]
  if (!file) return

  if (modelChangeListener) {
    modelChangeListener.dispose()
    modelChangeListener = null
  }

  if (!file.model) {
    file.model = monacoModule.editor.createModel(file.content, 'zeta')
    file.model.onDidChangeContent(() => {
      if (file.model) {
        file.content = file.model.getValue()
      }
    })
  }

  monacoInstance.setModel(file.model)
  monacoInstance.focus()

  modelChangeListener = monacoInstance.onDidChangeModelContent(() => {
    if (monacoInstance) {
      store.currentCode = monacoInstance.getValue()
    }
  })
}

function closeTab(index: number) {
  const file = openFiles.value[index]
  if (file?.model) {
    file.model.dispose()
  }
  openFiles.value.splice(index, 1)

  if (openFiles.value.length === 0) {
    store.currentCode = ''
    store.currentFile = null
    return
  }

  if (activeTab.value >= openFiles.value.length) {
    activeTab.value = openFiles.value.length - 1
  }
}

watch(activeTab, (newTab) => {
  if (openFiles.value.length > 0) {
    nextTick(() => attachModel(newTab))
  }
})

watch(currentFile, (path) => {
  if (!path) return
  const name = path.split('/').pop() || path.split('\\').pop() || path
  const existing = openFiles.value.findIndex(f => f.path === path)
  if (existing === -1) {
    openFiles.value.push({ name, path, content: currentCode.value || '', model: null })
    nextTick(() => attachModel(openFiles.value.length - 1))
    activeTab.value = openFiles.value.length - 1
  } else {
    activeTab.value = existing
  }
})
</script>
