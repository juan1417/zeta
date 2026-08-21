<template>
  <v-card flat tile style="height: 100%; overflow-y: auto">
    <v-data-table
      :headers="headers"
      :items="variableList"
      density="compact"
      hover
      fixed-header
      height="100%"
      :no-data-text="'No variables yet. Run a script.'"
    >
      <template #item.type="{ item }">
        <v-chip size="x-small" variant="tonal" color="primary">
          {{ item.type }}
        </v-chip>
      </template>

      <template #item.value="{ item }">
        <span class="text-truncate d-inline-block" style="max-width: 200px" :title="item.value">
          {{ item.value }}
        </span>
      </template>
    </v-data-table>
  </v-card>
</template>

<script setup lang="ts">
import { computed } from 'vue'
import { useZetaStore } from '../stores/zeta'
import { storeToRefs } from 'pinia'

const store = useZetaStore()
const { variables } = storeToRefs(store)

const headers = [
  { title: 'Name', key: 'name', sortable: true },
  { title: 'Type', key: 'type', sortable: true },
  { title: 'Value', key: 'value', sortable: false },
]

const variableList = computed(() =>
  Object.entries(variables.value).map(([name, info]) => ({
    name,
    type: info.type,
    value: info.value,
  }))
)
</script>
