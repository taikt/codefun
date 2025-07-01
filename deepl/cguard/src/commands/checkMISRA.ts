import * as vscode from 'vscode';
import { runRuleAnalysis } from '../helpers/copilot';

export function registerCheckMISRACommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.checkMISRARules', async () => {
    await runRuleAnalysis('MISRA');
  });
  context.subscriptions.push(disposable);
}
