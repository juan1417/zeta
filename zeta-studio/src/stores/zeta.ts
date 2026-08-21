import { defineStore } from 'pinia'
import { ref } from 'vue'
import { invoke } from '@tauri-apps/api/core'

export interface VariableInfo {
  type: string
  value: string
}

export const useZetaStore = defineStore('zeta', () => {
  const variables = ref<Record<string, VariableInfo>>({})
  const output = ref<string[]>([])
  const currentFile = ref<string | null>(null)
  const currentCode = ref<string>('')

  async function exec(code: string) {
    if (!code.trim()) {
      output.value.push('[info] No code to execute')
      return
    }
    try {
      const result = await invoke<{ success: boolean; output: string; error: string | null }>('exec_code', { code })
      if (result.output) {
        output.value.push(...result.output.split('\n').filter(Boolean))
      }
      if (result.error) {
        output.value.push(`[error] ${result.error}`)
      }
      await loadVariables()
    } catch (e) {
      output.value.push(`[error] ${e}`)
    }
  }

  async function saveFile() {
    if (!currentFile.value) return
    try {
      await invoke('write_file', { path: currentFile.value, content: currentCode.value })
      output.value.push(`[info] Saved ${currentFile.value}`)
    } catch (e) {
      output.value.push(`[error] Failed to save: ${e}`)
    }
  }

  async function loadVariables() {
    try {
      const arr = await invoke<Array<{ name: string; var_type: string; value: string }>>('get_variables')
      const record: Record<string, VariableInfo> = {}
      for (const v of arr) {
        record[v.name] = { type: v.var_type, value: v.value }
      }
      variables.value = record
    } catch {
      // silently ignore if backend not ready
    }
  }

  function clearOutput() {
    output.value = []
  }

  return { variables, output, currentFile, currentCode, exec, saveFile, loadVariables, clearOutput }
})
