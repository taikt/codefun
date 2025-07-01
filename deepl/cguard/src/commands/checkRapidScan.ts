import * as vscode from 'vscode';
import { runRuleAnalysis } from '../helpers/copilot';

export function registerCheckRapidScanCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkRapidScanRules', async () => {
    await runRuleAnalysis('RAPIDSCAN');
  });
  context.subscriptions.push(disposable);
}
