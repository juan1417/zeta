import { createApp } from "vue";
import { createPinia } from "pinia";
import { createVuetify } from "vuetify";
import * as components from "vuetify/components";
import * as directives from "vuetify/directives";
import "@mdi/font/css/materialdesignicons.css";
import "vuetify/styles";

import App from "./App.vue";

const vuetify = createVuetify({
  components,
  directives,
  theme: {
    defaultTheme: "dark",
    themes: {
      dark: {
        colors: {
          background: "#1e1e2e",
          surface: "#181825",
          primary: "#89b4fa",
          secondary: "#f38ba8",
        },
      },
    },
  },
});

const app = createApp(App);
app.use(createPinia());
app.use(vuetify);
app.mount("#app");
