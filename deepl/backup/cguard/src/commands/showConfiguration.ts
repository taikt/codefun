import * as vscode from 'vscode';

export function registerShowConfigurationCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.showConfiguration', async () => {
    const config = vscode.workspace.getConfiguration('lgedvCodeGuard');
    const reportDir = config.get<string>('reportDirectory') || '/home/worker/src/defect_report';
    const webServerDir = config.get<string>('webServerDirectory') || '/home/worker/src/codeguard/web_srv';
    const ruleSetDir = config.get<string>('ruleSetDirectory') || '/home/worker/src/codeguard/main_codeguard/src/rule_set';
    const autoOpen = config.get<boolean>('autoOpenBrowser') ?? true;
    const message = `📋 **LGEDV CodeGuard Configuration:**\n\n📁 **Report Directory:**\n\`${reportDir}\`\n\n🌐 **Web Server Directory:**\n\`${webServerDir}\`\n\n📖 **Rule Set Directory:**\n\`${ruleSetDir}\`\n\n🚀 **Auto Open Browser:** ${autoOpen ? '✅ Enabled' : '❌ Disabled'}\n\n💡 Use "⚙️ Open Settings" to modify these paths.`;
    await vscode.window.showInformationMessage(
      message,
      { modal: true },
      'Open Settings',
      'Open Report Directory'
    ).then(action => {
      if (action === 'Open Settings') {
        vscode.commands.executeCommand('workbench.action.openSettings', 'lgedvCodeGuard');
      } else if (action === 'Open Report Directory') {
        vscode.commands.executeCommand('vscode.openFolder', vscode.Uri.file(reportDir), { forceNewWindow: false });
      }
    });
  });
  context.subscriptions.push(disposable);
}
