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
    try {
      const result = await invoke<{ output: string; error: string | null }>('exec_code', { code })
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

  async function loadVariables() {
    try {
      variables.value = await invoke<Record<string, VariableInfo>>('get_variables')
    } catch {
      // silently ignore if backend not ready
    }
  }

  function clearOutput() {
    output.value = []
  }

  return { variables, output, currentFile, currentCode, exec, loadVariables, clearOutput }
})
