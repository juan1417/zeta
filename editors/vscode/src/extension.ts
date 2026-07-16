import * as path from 'path';
import { workspace, ExtensionContext } from 'vscode';
import {
    LanguageClient,
    LanguageClientOptions,
    ServerOptions,
} from 'vscode-languageclient/node';

let client: LanguageClient | undefined;

export function activate(context: ExtensionContext) {
    const serverCommand = workspace.getConfiguration('zeta').get<string>('lsp.path', 'zeta-lsp');

    const serverOptions: ServerOptions = {
        command: serverCommand,
        args: [],
        options: {
            env: { ...process.env },
        },
    };

    const clientOptions: LanguageClientOptions = {
        documentSelector: [
            { scheme: 'file', language: 'zeta' },
            { scheme: 'untitled', language: 'zeta' },
        ],
        synchronize: {
            fileEvents: workspace.createFileSystemWatcher('**/*.{zl,zeta}'),
        },
    };

    client = new LanguageClient('zeta', 'Zeta Language Server', serverOptions, clientOptions);
    client.start();
}

export function deactivate(): Thenable<void> | undefined {
    return client?.stop();
}
