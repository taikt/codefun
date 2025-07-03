import * as vscode from 'vscode';
import { runRuleAnalysis } from '../helpers/copilot';
// choose rules to check, it is disabled temporarily
export function registerInteractiveCheckCommand(context: vscode.ExtensionContext) {
  const disposable = vscode.commands.registerCommand('lgedv.interactiveCheck', async () => {
    const choice = await vscode.window.showQuickPick([
      { label: '🎯 LGEDV Rule Only', description: 'Check only LGEDV_CRCL rules (fast, focused)', detail: 'Best for LGEDV compliance check - faster analysis' },
      { label: '🛡️ MISRA C++ Rule Only', description: 'Check only MISRA C++ 2008 rules (fast, focused)', detail: 'Best for MISRA compliance check - faster analysis' },
      { label: '� CERT C++ Rule Only', description: 'Check only CERT C++ rules (fast, focused)', detail: 'Best for CERT compliance check - faster analysis' }
    ], {
      placeHolder: 'Select which rules to check against your code',
      title: 'LGEDV CodeGuard - Choose Analysis Type'
    });
    if (choice) {
      if (choice.label.includes('LGEDV')) await runRuleAnalysis('LGEDV');
      else if (choice.label.includes('MISRA')) await runRuleAnalysis('MISRA');
      else if (choice.label.includes('CERT')) await runRuleAnalysis('CERT');
    }
  });
  context.subscriptions.push(disposable);
}
