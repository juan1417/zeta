import { invoke } from "@tauri-apps/api/core";

export interface CompletionItem {
  label: string;
  kind: number;
  detail?: string;
  documentation?: string;
  insertText?: string;
}

export interface HoverInfo {
  contents: string;
  range?: {
    startLine: number;
    startCharacter: number;
    endLine: number;
    endCharacter: number;
  };
}

export interface LspLocation {
  uri: string;
  line: number;
  character: number;
}

class ZetaLspClient {
  private static instance: ZetaLspClient;
  private initialized = false;

  private constructor() {}

  static getInstance(): ZetaLspClient {
    if (!ZetaLspClient.instance) {
      ZetaLspClient.instance = new ZetaLspClient();
    }
    return ZetaLspClient.instance;
  }

  async initialize(workspacePath: string): Promise<boolean> {
    try {
      this.initialized = await invoke<boolean>("lsp_initialize", {
        workspacePath,
      });
      return this.initialized;
    } catch (e) {
      console.error("LSP initialize failed:", e);
      this.initialized = false;
      return false;
    }
  }

  async didOpen(
    uri: string,
    content: string,
    languageId: string = "zeta",
    version: number = 1
  ): Promise<void> {
    if (!this.initialized) return;
    try {
      await invoke("lsp_did_open", { uri, content, languageId, version });
    } catch (e) {
      console.error("LSP didOpen failed:", e);
    }
  }

  async didChange(uri: string, content: string, version: number): Promise<void> {
    if (!this.initialized) return;
    try {
      await invoke("lsp_did_change", { uri, content, version });
    } catch (e) {
      console.error("LSP didChange failed:", e);
    }
  }

  async completion(
    uri: string,
    line: number,
    character: number
  ): Promise<CompletionItem[]> {
    if (!this.initialized) return [];
    try {
      return await invoke<CompletionItem[]>("lsp_completion", {
        uri,
        line,
        character,
      });
    } catch (e) {
      console.error("LSP completion failed:", e);
      return [];
    }
  }

  async hover(uri: string, line: number, character: number): Promise<HoverInfo | null> {
    if (!this.initialized) return null;
    try {
      return await invoke<HoverInfo | null>("lsp_hover", { uri, line, character });
    } catch (e) {
      console.error("LSP hover failed:", e);
      return null;
    }
  }

  async definition(
    uri: string,
    line: number,
    character: number
  ): Promise<LspLocation | null> {
    if (!this.initialized) return null;
    try {
      return await invoke<LspLocation | null>("lsp_definition", {
        uri,
        line,
        character,
      });
    } catch (e) {
      console.error("LSP definition failed:", e);
      return null;
    }
  }

  async shutdown(): Promise<void> {
    if (!this.initialized) return;
    try {
      await invoke("lsp_shutdown");
    } catch (e) {
      console.error("LSP shutdown failed:", e);
    }
    this.initialized = false;
  }

  isInitialized(): boolean {
    return this.initialized;
  }
}

export const zetaLsp = ZetaLspClient.getInstance();
