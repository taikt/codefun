// https://codepro.lge.com/exam/19/overseas-questions-for-previous-test/quiz/7%3Fembed
// AC
// change Touch()
int dr[8] = { -1,-1,0,1,1, 1, 0, -1};
int dc[8] = { 0,  1,1,1,0, -1,-1,-1};

void Touch(int r, int c) {
	int V;

	if (A[r][c] == 0) V = 1;
	else V = 0;
	A[r][c] = V;

	for (int k = 0; k < 8; k++) {
		int nr = r;
		int nc = c;
		int flag = 0;
		bool bom=false;

		for (;;) {
			nr = nr + dr[k];
			nc = nc + dc[k];
			if (nr < 0 || nr >= H || nc < 0 || nc >= W) break;
			if (A[nr][nc]==2) bom=true;
			if (A[nr][nc] == V) {
				flag = 1;
				break;
			}
		}
		if (flag == 1 && !bom) {
			nr = r;
			nc = c;
			for (;;) {
				nr = nr + dr[k];
				nc = nc + dc[k];
				if (A[nr][nc] == V) {
					break;
				}
				A[nr][nc] = V;
			}
		} else if (flag==1 && bom){
			nr = r;
			nc = c;
			for (;;) {
				nr = nr + dr[k];
				nc = nc + dc[k];
				if (nr < 0 || nr >= H || nc < 0 || nc >= W) break;
				A[nr][nc] = V;
			}
		}
	}
}