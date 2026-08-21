use std::collections::HashMap;
use std::fs;
use std::process::Command;
use std::sync::Mutex;
use tauri::State;

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

                vars.push(VariableInfo {
                    name,
                    var_type,
                    value: raw_value,
                });
            }
        }
    }
    vars
}

#[tauri::command]
fn exec_code(
    code: String,
    state: State<AppState>,
    zeta_bin: String,
) -> ExecResult {
    let temp_dir = std::env::temp_dir();
    let temp_file = temp_dir.join("zeta_eval.zl");

    if let Err(e) = fs::write(&temp_file, &code) {
        return ExecResult {
            success: false,
            output: String::new(),
            error: Some(format!("Failed to write temp file: {}", e)),
        };
    }

    let result = Command::new(&zeta_bin)
        .arg(temp_file.to_str().unwrap_or_default())
        .output();

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

            ExecResult {
                success: out.status.success(),
                output: stdout,
                error: if stderr.is_empty() { None } else { Some(stderr) },
            }
        }
        Err(e) => ExecResult {
            success: false,
            output: String::new(),
            error: Some(e.to_string()),
        },
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
