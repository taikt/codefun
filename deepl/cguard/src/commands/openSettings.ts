import * as vscode from 'vscode';
export function registerOpenSettingsCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.openSettings', async () => {
    await vscode.commands.executeCommand('workbench.action.openSettings', 'lgedvCodeGuard');
  });
  context.subscriptions.push(disposable);
}
