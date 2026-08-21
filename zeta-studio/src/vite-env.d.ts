/// <reference types="vite/client" />

declare module "*.vue" {
  import type { DefineComponent } from "vue";
  const component: DefineComponent<{}, {}, any>;
  export default component;
}

interface TauriInvoke {
  (cmd: string, args?: Record<string, unknown>): Promise<unknown>;
}

interface TauriListen {
  (event: string, handler: (event: { payload: unknown }) => void): Promise<() => void>;
}

interface TauriWindow {
  __TAURI__?: {
    invoke: TauriInvoke;
    listen: TauriListen;
    event?: {
      listen: TauriListen;
      emit: (event: string, payload?: unknown) => Promise<void>;
    };
    window?: {
      appWindow?: {
        close: () => Promise<void>;
        minimize: () => Promise<void>;
        toggleMaximize: () => Promise<void>;
      };
    };
  };
}

declare interface Window extends TauriWindow {}
