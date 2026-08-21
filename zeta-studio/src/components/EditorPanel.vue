<template>
  <v-card flat tile style="height: 100%">
    <v-tabs v-model="activeTab" density="compact" color="primary" show-arrows>
      <v-tab v-for="(file, i) in openFiles" :key="file.path" :value="i">
        <span class="text-body-2">{{ file.name }}</span>
        <v-icon v-if="file.modified" size="x-small" color="amber" class="ml-1">mdi-circle</v-icon>
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
import type { editor as MonacoEditor } from 'monaco-editor'
import { invoke } from '@tauri-apps/api/core'
import { useZetaStore } from '../stores/zeta'
import { storeToRefs } from 'pinia'
import { zetaLanguage, zetaMonarch } from '../zeta-language'

const store = useZetaStore()
const { currentFile } = storeToRefs(store)

const editorContainer = ref<HTMLElement | null>(null)
const activeTab = ref(0)
const openFiles = ref<{
  name: string
  path: string
  content: string
  savedContent: string
  modified: boolean
  model: MonacoEditor.ITextModel | null
}[]>([])

let monacoInstance: MonacoEditor.IStandaloneCodeEditor | null = null
let monacoModule: typeof import('monaco-editor') | null = null

function getCode(): string {
  if (monacoInstance) {
    return monacoInstance.getValue()
  }
  return ''
}

function updateModifiedFlag() {
  const idx = activeTab.value
  const file = openFiles.value[idx]
  if (!file) return
  const currentCode = getCode()
  file.modified = currentCode !== file.savedContent
}

async function saveCurrentFile() {
  const idx = activeTab.value
  const file = openFiles.value[idx]
  if (!file) {
    store.output.push('[error] No file open to save')
    return
  }
  const code = getCode()
  if (!code && code !== '') {
    store.output.push('[error] No code to save')
    return
  }
  try {
    await invoke('write_file', { path: file.path, content: code })
    file.savedContent = code
    file.content = code
    file.modified = false
    store.output.push(`[info] Saved ${file.path}`)
  } catch (e) {
    store.output.push(`[error] Failed to save: ${e}`)
  }
}

async function runCode() {
  const code = getCode()
  if (!code.trim()) {
    store.output.push('[info] No code to execute')
    return
  }
  await store.exec(code)
}

onMounted(async () => {
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

  // NOW set __zeta AFTER Monaco is ready
  ;(window as any).__zeta = { getCode, saveCurrentFile, runCode }

  monacoInstance.addCommand(monacoModule.KeyMod.CtrlCmd | monacoModule.KeyCode.Enter, () => {
    runCode()
  })
  monacoInstance.addCommand(monacoModule.KeyMod.CtrlCmd | monacoModule.KeyCode.KeyS, () => {
    saveCurrentFile()
  })

  // Track modifications
  monacoInstance.onDidChangeModelContent(() => {
    updateModifiedFlag()
  })
})

onBeforeUnmount(() => {
  openFiles.value.forEach(f => f.model?.dispose())
  monacoInstance?.dispose()
  delete (window as any).__zeta
})

function attachModel(index: number) {
  if (!monacoInstance || !monacoModule) return
  const file = openFiles.value[index]
  if (!file) return

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
  updateModifiedFlag()
}

function closeTab(index: number) {
  const file = openFiles.value[index]
  if (file?.model) {
    file.model.dispose()
  }
  openFiles.value.splice(index, 1)

  if (openFiles.value.length === 0) {
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
    const content = (window as any).__zeta_pending_content || ''
    openFiles.value.push({
      name,
      path,
      content,
      savedContent: content,
      modified: false,
      model: null,
    })
    delete (window as any).__zeta_pending_content
    activeTab.value = openFiles.value.length - 1
  } else {
    activeTab.value = existing
  }
})
</script>
