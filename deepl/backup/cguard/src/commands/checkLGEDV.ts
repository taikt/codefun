import * as vscode from 'vscode';
import { runRuleAnalysis } from '../helpers/copilot';

export function registerCheckLGEDVCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkLGEDVRules', async () => {
    await runRuleAnalysis('LGEDV');
  });
  context.subscriptions.push(disposable);
}
