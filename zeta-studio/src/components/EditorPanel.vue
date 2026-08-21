<template>
  <v-card flat tile style="height: 100%">
    <v-tabs v-model="activeTab" density="compact" color="primary">
      <v-tab v-for="(file, i) in openFiles" :key="i" :value="i">
        {{ file.name }}
      </v-tab>
    </v-tabs>
    <div ref="editorContainer" style="height: calc(100% - 40px)"></div>
  </v-card>
</template>

<script setup lang="ts">
import { ref, watch, onMounted, onBeforeUnmount } from 'vue'
import editor from '@monaco-editor/loader'
import type { editor as MonacoEditor } from 'monaco-editor'
import { useZetaStore } from '../stores/zeta'
import { storeToRefs } from 'pinia'
import { zetaLanguage, zetaMonarch } from '../zeta-language'

const store = useZetaStore()
const { currentFile, currentCode } = storeToRefs(store)

const editorContainer = ref<HTMLElement | null>(null)
const activeTab = ref(0)
const openFiles = ref<{ name: string; path: string; content: string }[]>([])

let monacoInstance: MonacoEditor.IStandaloneCodeEditor | null = null

onMounted(async () => {
  const monaco = await editor.init()
  if (!editorContainer.value) return

  monaco.languages.register({ id: 'zeta' })
  monaco.languages.setLanguageConfiguration('zeta', zetaLanguage)
  monaco.languages.setMonarchTokensProvider('zeta', zetaMonarch)

  monacoInstance = monaco.editor.create(editorContainer.value, {
    value: currentCode.value || '',
    language: 'zeta',
    theme: 'vs-dark',
    minimap: { enabled: false },
    fontSize: 14,
    automaticLayout: true,
    scrollBeyondLastLine: false,
    padding: { top: 8 },
  })

  if (monacoInstance) {
    monacoInstance.onDidChangeModelContent(() => {
      if (monacoInstance) {
        store.currentCode = monacoInstance.getValue()
      }
    })

    // Ctrl+Enter to run
    monacoInstance.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter, () => {
      window.dispatchEvent(new CustomEvent('zeta-run'))
    })

    // Ctrl+S to save
    monacoInstance.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KeyS, () => {
      window.dispatchEvent(new CustomEvent('zeta-save'))
    })
  }
})

watch(currentCode, (val) => {
  if (monacoInstance && val !== monacoInstance.getValue()) {
    monacoInstance.setValue(val || '')
  }
})

watch(currentFile, (path) => {
  if (!path) return
  const name = path.split('/').pop() || path.split('\\').pop() || path
  const existing = openFiles.value.find(f => f.path === path)
  if (!existing) {
    openFiles.value.push({ name, path, content: currentCode.value || '' })
    activeTab.value = openFiles.value.length - 1
  } else {
    activeTab.value = openFiles.value.indexOf(existing)
  }
})

onBeforeUnmount(() => {
  monacoInstance?.dispose()
})
</script>
