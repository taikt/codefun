import * as vscode from 'vscode';
import { runRuleAnalysis } from '../helpers/copilot';

export function registerCheckCERTCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkCERTRules', async () => {
    await runRuleAnalysis('CERT');
  });
  context.subscriptions.push(disposable);
}
