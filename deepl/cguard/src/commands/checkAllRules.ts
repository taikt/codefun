import * as vscode from 'vscode';
import { runRuleAnalysis } from '../helpers/copilot';

export function registerCheckAllRulesCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkAllRules', async () => {
    await runRuleAnalysis('CRITICAL');
  });
  context.subscriptions.push(disposable);
}
