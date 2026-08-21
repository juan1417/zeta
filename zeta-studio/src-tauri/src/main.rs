use std::collections::HashMap;
use std::fs;
use std::path::PathBuf;
use std::process::Command;
use std::sync::Mutex;
use tauri::State;
use tauri_plugin_dialog::DialogExt;
use tokio::sync::oneshot;

mod lsp_bridge;

struct AppState {
    variables: Mutex<HashMap<String, VariableInfo>>,
    output: Mutex<Vec<String>>,
}

#[derive(serde::Serialize, Clone, Debug)]
struct VariableInfo {
    name: String,
    var_type: String,
    value: String,
}

#[derive(serde::Serialize)]
struct ExecResult {
    success: bool,
    output: String,
    error: Option<String>,
}

#[derive(serde::Serialize, Clone, Debug)]
struct FileInfo {
    name: String,
    path: String,
    is_dir: bool,
    extension: String,
}

fn find_zeta_binary() -> Option<String> {
    if let Ok(output) = Command::new("which").arg("zeta").output() {
        if output.status.success() {
            let path = String::from_utf8_lossy(&output.stdout).trim().to_string();
            if !path.is_empty() {
                return Some(path);
            }
        }
    }
    let home = std::env::var("HOME").unwrap_or_default();
    let candidates = [
        format!("{}/.local/bin/zeta", home),
        "/usr/local/bin/zeta".to_string(),
        "./zeta".to_string(),
    ];
    for p in &candidates {
        if PathBuf::from(p).exists() {
            return Some(p.clone());
        }
    }
    None
}

fn parse_variables(stdout: &str) -> Vec<VariableInfo> {
    let mut vars = Vec::new();
    for line in stdout.lines() {
        let trimmed = line.trim();
        if trimmed.starts_with('$') {
            if let Some(eq_pos) = trimmed.find('=') {
                let name = trimmed[..eq_pos].trim().to_string();
                let raw_value = trimmed[eq_pos + 1..].trim().to_string();
                let var_type = if raw_value.starts_with('"') || raw_value.starts_with('\'') {
                    "string".to_string()
                } else if raw_value == "true" || raw_value == "false" {
                    "bool".to_string()
                } else if raw_value.starts_with('[') || raw_value.starts_with('<') {
                    "vector".to_string()
                } else if raw_value == "null" {
                    "null".to_string()
                } else if raw_value.parse::<f64>().is_ok() {
                    "number".to_string()
                } else {
                    "unknown".to_string()
                };
                vars.push(VariableInfo { name, var_type, value: raw_value });
            }
        }
    }
    vars
}

#[tauri::command]
async fn exec_code(code: String, state: State<'_, AppState>) -> Result<ExecResult, String> {
    let code = code.clone();
    let zeta_bin = match find_zeta_binary() {
        Some(b) => b,
        None => {
            return Ok(ExecResult {
                success: false,
                output: String::new(),
                error: Some("Zeta binary not found. Ensure 'zeta' is in PATH.".to_string()),
            });
        }
    };

    let temp_dir = std::env::temp_dir();
    let temp_file = temp_dir.join("zeta_eval.zl");

    if let Err(e) = fs::write(&temp_file, &code) {
        return Ok(ExecResult {
            success: false,
            output: String::new(),
            error: Some(format!("Failed to write temp file: {}", e)),
        });
    }

    let result = tokio::process::Command::new(&zeta_bin)
        .arg(temp_file.to_str().unwrap_or_default())
        .output()
        .await;

    let _ = fs::remove_file(&temp_file);

    match result {
        Ok(out) => {
            let stdout = String::from_utf8_lossy(&out.stdout).to_string();
            let stderr = String::from_utf8_lossy(&out.stderr).to_string();

            let new_vars = parse_variables(&stdout);
            {
                let mut vars = state.variables.lock().unwrap();
                for v in new_vars {
                    vars.insert(v.name.clone(), v);
                }
            }
            {
                let mut output_lock = state.output.lock().unwrap();
                for line in stdout.lines() {
                    output_lock.push(line.to_string());
                }
                if !stderr.is_empty() {
                    output_lock.push(format!("ERROR: {}", stderr));
                }
            }

            Ok(ExecResult {
                success: out.status.success(),
                output: stdout,
                error: if stderr.is_empty() { None } else { Some(stderr) },
            })
        }
        Err(e) => Ok(ExecResult {
            success: false,
            output: String::new(),
            error: Some(e.to_string()),
        }),
    }
}

#[tauri::command]
async fn open_folder_dialog(app: tauri::AppHandle) -> Result<String, String> {
    let (tx, rx) = oneshot::channel::<Option<String>>();
    app.dialog().file().pick_folder(move |path| {
        let result = path.map(|p| p.to_string());
        let _ = tx.send(result);
    });
    match rx.await {
        Ok(Some(path)) => Ok(path),
        Ok(None) => Err("No folder selected".to_string()),
        Err(e) => Err(format!("Channel error: {}", e)),
    }
}

#[tauri::command]
fn list_directory(path: String) -> Result<Vec<FileInfo>, String> {
    let dir = std::path::Path::new(&path);
    if !dir.is_dir() {
        return Err(format!("Not a directory: {}", path));
    }
    let mut entries = fs::read_dir(dir).map_err(|e| e.to_string())?;
    let mut files = Vec::new();
    while let Some(Ok(entry)) = entries.next() {
        let metadata = match entry.metadata() {
            Ok(m) => m,
            Err(_) => continue,
        };
        let name = entry.file_name().to_string_lossy().to_string();
        let entry_path = entry.path().to_string_lossy().to_string();
        let is_dir = metadata.is_dir();
        let extension = if is_dir {
            String::new()
        } else {
            std::path::Path::new(&name)
                .extension()
                .map(|e| e.to_string_lossy().to_string())
                .unwrap_or_default()
        };
        // Skip hidden files
        if name.starts_with('.') {
            continue;
        }
        files.push(FileInfo { name, path: entry_path, is_dir, extension });
    }
    files.sort_by(|a, b| {
        if a.is_dir != b.is_dir {
            b.is_dir.cmp(&a.is_dir) // dirs first
        } else {
            a.name.to_lowercase().cmp(&b.name.to_lowercase())
        }
    });
    Ok(files)
}

#[tauri::command]
fn read_file(path: String) -> Result<String, String> {
    fs::read_to_string(&path).map_err(|e| format!("Failed to read file: {}", e))
}

#[tauri::command]
fn write_file(path: String, content: String) -> Result<(), String> {
    fs::write(&path, content).map_err(|e| format!("Failed to write file: {}", e))
}

#[tauri::command]
async fn save_file_dialog(app: tauri::AppHandle, default_name: String) -> Result<String, String> {
    let (tx, rx) = oneshot::channel::<Option<String>>();
    app.dialog()
        .file()
        .set_file_name(&default_name)
        .add_filter("Zeta files", &["zl", "zeta"])
        .add_filter("All files", &["*"])
        .save_file(move |path| {
            let result = path.map(|p| p.to_string());
            let _ = tx.send(result);
        });
    match rx.await {
        Ok(Some(path)) => Ok(path),
        Ok(None) => Err("No file path selected".to_string()),
        Err(e) => Err(format!("Channel error: {}", e)),
    }
}

#[tauri::command]
fn get_variables(state: State<AppState>) -> Vec<VariableInfo> {
    let vars = state.variables.lock().unwrap();
    vars.values().cloned().collect()
}

#[tauri::command]
fn get_output(state: State<AppState>) -> Vec<String> {
    let output = state.output.lock().unwrap();
    output.clone()
}

#[tauri::command]
fn clear_output(state: State<AppState>) {
    let mut output = state.output.lock().unwrap();
    output.clear();
}

#[tauri::command]
fn get_version() -> String {
    env!("CARGO_PKG_VERSION").to_string()
}

fn main() {
    tauri::Builder::default()
        .plugin(tauri_plugin_dialog::init())
        .plugin(tauri_plugin_fs::init())
        .manage(AppState {
            variables: Mutex::new(HashMap::new()),
            output: Mutex::new(Vec::new()),
        })
        .manage(Mutex::new(lsp_bridge::LspState::new()))
        .invoke_handler(tauri::generate_handler![
            exec_code,
            get_variables,
            get_output,
            clear_output,
            get_version,
            open_folder_dialog,
            list_directory,
            read_file,
            write_file,
            save_file_dialog,
            lsp_bridge::lsp_initialize,
            lsp_bridge::lsp_did_open,
            lsp_bridge::lsp_did_change,
            lsp_bridge::lsp_completion,
            lsp_bridge::lsp_hover,
            lsp_bridge::lsp_definition,
            lsp_bridge::lsp_shutdown,
        ])
        .run(tauri::generate_context!())
        .expect("error while running Zeta Studio");
}
