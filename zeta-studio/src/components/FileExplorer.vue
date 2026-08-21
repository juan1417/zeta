<template>
  <v-card flat tile style="height: 100%">
    <v-toolbar density="compact" color="primary">
      <v-toolbar-title class="text-subtitle-1">Explorer</v-toolbar-title>
      <v-spacer />
      <v-btn icon="mdi-folder-open" size="small" @click="openFolder" title="Open Folder" />
      <v-btn icon="mdi-file-plus" size="small" @click="newFile" title="New File" />
    </v-toolbar>
    <v-list density="compact" nav>
      <template v-if="rootPath">
        <div class="text-caption pa-2 text-medium-emphasis">{{ rootPath }}</div>
        <v-list-item
          v-for="file in files"
          :key="file.path"
          :title="file.name"
          :subtitle="file.isDir ? 'folder' : file.extension"
          :prepend-icon="file.isDir ? 'mdi-folder' : 'mdi-file-code'"
          @click="file.isDir ? toggleDir(file) : openFile(file)"
          density="compact"
        />
      </template>
      <v-list-item v-else>
        <div class="text-center text-medium-emphasis pa-4">
          <v-icon icon="mdi-folder-open-outline" size="48" />
          <div class="mt-2">Open a folder to browse files</div>
        </div>
      </v-list-item>
    </v-list>
  </v-card>
</template>

<script setup lang="ts">
import { ref } from 'vue'
import { invoke } from '@tauri-apps/api/core'

const emit = defineEmits<{
  (e: 'open-file', path: string): void
}>()

interface FileInfo {
  name: string
  path: string
  isDir: boolean
  extension: string
}

const rootPath = ref('')
const files = ref<FileInfo[]>([])

async function openFolder() {
  try {
    const path = await invoke<string>('open_folder_dialog')
    if (path) {
      rootPath.value = path
      await loadDirectory(path)
    }
  } catch (e) {
    console.error('Failed to open folder:', e)
  }
}

async function loadDirectory(path: string) {
  try {
    files.value = await invoke<FileInfo[]>('list_directory', { path })
  } catch (e) {
    console.error('Failed to list directory:', e)
  }
}

function toggleDir(file: FileInfo) {
  loadDirectory(file.path)
}

function openFile(file: FileInfo) {
  emit('open-file', file.path)
}

async function newFile() {
  try {
    const path = await invoke<string>('save_file_dialog', { defaultName: 'untitled.zl' })
    if (path) {
      await invoke('write_file', { path, content: '' })
      emit('open-file', path)
      if (rootPath.value) {
        await loadDirectory(rootPath.value)
      }
    }
  } catch (e) {
    console.error('Failed to create file:', e)
  }
}
</script>
