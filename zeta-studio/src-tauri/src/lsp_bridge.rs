use serde::{Deserialize, Serialize};
use std::collections::HashMap;
use std::io::{BufRead, BufReader, Read, Write};
use std::process::{Child, ChildStdin, ChildStdout, Command, Stdio};
use std::sync::Mutex;
use tauri::State;

pub struct LspState {
    child: Option<Child>,
    stdin: Option<ChildStdin>,
    stdout: Option<ChildStdout>,
    next_id: u64,
    pending: HashMap<u64, serde_json::Value>,
}

impl LspState {
    fn new() -> Self {
        Self {
            child: None,
            stdin: None,
            stdout: None,
            next_id: 1,
            pending: HashMap::new(),
        }
    }

    fn send_request(
        &mut self,
        method: &str,
        params: serde_json::Value,
    ) -> Result<u64, String> {
        let id = self.next_id;
        self.next_id += 1;

        let request = serde_json::json!({
            "jsonrpc": "2.0",
            "id": id,
            "method": method,
            "params": params,
        });

        self.send_message(&request)?;
        self.pending.insert(id, request);
        Ok(id)
    }

    fn send_notification(
        &mut self,
        method: &str,
        params: serde_json::Value,
    ) -> Result<(), String> {
        let notification = serde_json::json!({
            "jsonrpc": "2.0",
            "method": method,
            "params": params,
        });

        self.send_message(&notification)
    }

    fn send_message(&mut self, msg: &serde_json::Value) -> Result<(), String> {
        let body = serde_json::to_string(msg).map_err(|e| e.to_string())?;
        let content_length = body.len();

        let stdin = self
            .stdin
            .as_mut()
            .ok_or("LSP process not running")?;

        write!(stdin, "Content-Length: {}\r\n\r\n{}", content_length, body)
            .map_err(|e| e.to_string())?;
        stdin.flush().map_err(|e| e.to_string())?;
        Ok(())
    }

    fn read_message(&mut self) -> Result<serde_json::Value, String> {
        let stdout = self
            .stdout
            .as_mut()
            .ok_or("LSP process not running")?;

        let mut reader = BufReader::new(stdout);
        let mut content_length: usize = 0;

        loop {
            let mut header_line = String::new();
            reader
                .read_line(&mut header_line)
                .map_err(|e| e.to_string())?;

            let header_line = header_line.trim();
            if header_line.is_empty() {
                break;
            }

            if let Some(val) = header_line.strip_prefix("Content-Length: ") {
                content_length = val.parse().map_err(|e| format!("bad Content-Length: {}", e))?;
            }
        }

        if content_length == 0 {
            return Err("No Content-Length header".to_string());
        }

        let mut body = vec![0u8; content_length];
        reader
            .read_exact(&mut body)
            .map_err(|e| e.to_string())?;

        let msg: serde_json::Value =
            serde_json::from_slice(&body).map_err(|e| e.to_string())?;
        Ok(msg)
    }
}

#[tauri::command]
pub fn lsp_initialize(
    workspace_path: String,
    state: State<Mutex<LspState>>,
) -> Result<bool, String> {
    let mut st = state.lock().map_err(|e| e.to_string())?;

    // Kill existing process if any
    if let Some(ref mut child) = st.child {
        let _ = child.kill();
        let _ = child.wait();
    }

    let mut child = Command::new("zeta-lsp")
        .arg("--stdio")
        .arg(&workspace_path)
        .stdin(Stdio::piped())
        .stdout(Stdio::piped())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("Failed to spawn zeta-lsp: {}", e))?;

    let stdin = child.stdin.take().ok_or("Failed to capture stdin")?;
    let stdout = child.stdout.take().ok_or("Failed to capture stdout")?;

    st.child = Some(child);
    st.stdin = Some(stdin);
    st.stdout = Some(stdout);
    st.next_id = 1;
    st.pending.clear();

    // Send initialize request
    let init_params = serde_json::json!({
        "processId": std::process::id(),
        "rootUri": format!("file://{}", workspace_path),
        "capabilities": {
            "textDocument": {
                "completion": {
                    "completionItem": {
                        "snippetSupport": false
                    }
                },
                "hover": {},
                "definition": {}
            }
        }
    });

    let id = st.send_request("initialize", init_params)?;

    // Read response
    let response = st.read_message()?;

    // Check if it's a response to our request
    if let Some(resp_id) = response.get("id").and_then(|v| v.as_u64()) {
        if resp_id == id {
            st.pending.remove(&id);
        }
    }

    // Send initialized notification
    st.send_notification("initialized", serde_json::json!({}))?;

    Ok(true)
}

#[tauri::command]
pub fn lsp_did_open(
    uri: String,
    content: String,
    language_id: String,
    version: i32,
    state: State<Mutex<LspState>>,
) -> Result<(), String> {
    let mut st = state.lock().map_err(|e| e.to_string())?;

    st.send_notification(
        "textDocument/didOpen",
        serde_json::json!({
            "textDocument": {
                "uri": uri,
                "languageId": language_id,
                "version": version,
                "text": content,
            }
        }),
    )
}

#[tauri::command]
pub fn lsp_did_change(
    uri: String,
    content: String,
    version: i32,
    state: State<Mutex<LspState>>,
) -> Result<(), String> {
    let mut st = state.lock().map_err(|e| e.to_string())?;

    st.send_notification(
        "textDocument/didChange",
        serde_json::json!({
            "textDocument": {
                "uri": uri,
                "version": version,
            },
            "contentChanges": [{
                "text": content,
            }]
        }),
    )
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct CompletionItem {
    pub label: String,
    #[serde(rename = "kind")]
    pub kind: i32,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub detail: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub documentation: Option<String>,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub insert_text: Option<String>,
}

#[tauri::command]
pub fn lsp_completion(
    uri: String,
    line: i32,
    character: i32,
    state: State<Mutex<LspState>>,
) -> Result<Vec<CompletionItem>, String> {
    let mut st = state.lock().map_err(|e| e.to_string())?;

    let id = st.send_request(
        "textDocument/completion",
        serde_json::json!({
            "textDocument": { "uri": uri },
            "position": { "line": line, "character": character }
        }),
    )?;

    let response = st.read_message()?;
    st.pending.remove(&id);

    let items = response
        .get("result")
        .and_then(|r| r.get("items"))
        .and_then(|items| items.as_array())
        .map(|arr| {
            arr.iter()
                .filter_map(|item| serde_json::from_value(item.clone()).ok())
                .collect()
        })
        .unwrap_or_default();

    Ok(items)
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct HoverInfo {
    pub contents: String,
    #[serde(skip_serializing_if = "Option::is_none")]
    pub range: Option<HoverRange>,
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct HoverRange {
    pub start_line: i32,
    pub start_character: i32,
    pub end_line: i32,
    pub end_character: i32,
}

#[tauri::command]
pub fn lsp_hover(
    uri: String,
    line: i32,
    character: i32,
    state: State<Mutex<LspState>>,
) -> Result<Option<HoverInfo>, String> {
    let mut st = state.lock().map_err(|e| e.to_string())?;

    let id = st.send_request(
        "textDocument/hover",
        serde_json::json!({
            "textDocument": { "uri": uri },
            "position": { "line": line, "character": character }
        }),
    )?;

    let response = st.read_message()?;
    st.pending.remove(&id);

    match response.get("result") {
        Some(serde_json::Value::Null) | None => Ok(None),
        Some(result) => {
            let contents = match result.get("contents") {
                Some(serde_json::Value::String(s)) => s.clone(),
                Some(serde_json::Value::Object(obj)) => {
                    if let Some(val) = obj.get("value") {
                        val.as_str().unwrap_or("").to_string()
                    } else {
                        serde_json::to_string(obj).unwrap_or_default()
                    }
                }
                Some(serde_json::Value::Array(arr)) => arr
                    .iter()
                    .filter_map(|v| v.as_str())
                    .collect::<Vec<_>>()
                    .join("\n"),
                _ => serde_json::to_string(result).unwrap_or_default(),
            };

            let range = result.get("range").and_then(|r| {
                let start = r.get("start")?;
                let end = r.get("end")?;
                Some(HoverRange {
                    start_line: start.get("line")?.as_i32()?,
                    start_character: start.get("character")?.as_i32()?,
                    end_line: end.get("line")?.as_i32()?,
                    end_character: end.get("character")?.as_i32()?,
                })
            });

            Ok(Some(HoverInfo { contents, range }))
        }
    }
}

#[derive(Serialize, Deserialize, Clone, Debug)]
pub struct LspLocation {
    pub uri: String,
    pub line: i32,
    pub character: i32,
}

#[tauri::command]
pub fn lsp_definition(
    uri: String,
    line: i32,
    character: i32,
    state: State<Mutex<LspState>>,
) -> Result<Option<LspLocation>, String> {
    let mut st = state.lock().map_err(|e| e.to_string())?;

    let id = st.send_request(
        "textDocument/definition",
        serde_json::json!({
            "textDocument": { "uri": uri },
            "position": { "line": line, "character": character }
        }),
    )?;

    let response = st.read_message()?;
    st.pending.remove(&id);

    match response.get("result") {
        Some(serde_json::Value::Null) | None => Ok(None),
        Some(serde_json::Value::Array(arr)) => {
            if let Some(loc) = arr.first() {
                let uri = loc
                    .get("uri")
                    .and_then(|u| u.as_str())
                    .unwrap_or("")
                    .to_string();
                let pos = loc.get("range").and_then(|r| r.get("start"))?;
                let line = pos.get("line").and_then(|l| l.as_i32()).unwrap_or(0);
                let character = pos
                    .get("character")
                    .and_then(|c| c.as_i32())
                    .unwrap_or(0);
                Ok(Some(LspLocation {
                    uri,
                    line,
                    character,
                }))
            } else {
                Ok(None)
            }
        }
        Some(serde_json::Value::Object(_)) => {
            let uri = response
                .get("uri")
                .and_then(|u| u.as_str())
                .unwrap_or("")
                .to_string();
            let pos = response
                .get("range")
                .and_then(|r| r.get("start"))?;
            let line = pos.get("line").and_then(|l| l.as_i32()).unwrap_or(0);
            let character = pos
                .get("character")
                .and_then(|c| c.as_i32())
                .unwrap_or(0);
            Ok(Some(LspLocation {
                uri,
                line,
                character,
            }))
        }
        _ => Ok(None),
    }
}

#[tauri::command]
pub fn lsp_shutdown(state: State<Mutex<LspState>>) -> Result<(), String> {
    let mut st = state.lock().map_err(|e| e.to_string())?;

    let _ = st.send_request("shutdown", serde_json::json!({}));
    let _ = st.read_message();
    let _ = st.send_notification("exit", serde_json::json!({}));

    if let Some(ref mut child) = st.child {
        let _ = child.kill();
        let _ = child.wait();
    }

    st.child = None;
    st.stdin = None;
    st.stdout = None;
    st.pending.clear();

    Ok(())
}
