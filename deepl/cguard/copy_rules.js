const { execSync } = require('child_process');
const isWin = process.platform === 'win32';
if (isWin) {
  execSync('xcopy src\\rule_set out\\rule_set /E /I /Y', { stdio: 'inherit' });
} else {
  execSync('cp -rf src/rule_set out/rule_set', { stdio: 'inherit' });
}