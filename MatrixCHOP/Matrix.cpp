class Mat4 {

public:
	float vals[4][4];

	Mat4() {
		identity();
	}

	void identity() {
		vals[0][0] = 1.;
		vals[0][1] = 0.;
		vals[0][2] = 0.;
		vals[0][3] = 0.;

		vals[1][0] = 0.;
		vals[1][1] = 1.;
		vals[1][2] = 0.;
		vals[1][3] = 0.;

		vals[2][0] = 0.;
		vals[2][1] = 0.;
		vals[2][2] = 1.;
		vals[2][3] = 0.;

		vals[3][0] = 0.;
		vals[3][1] = 0.;
		vals[3][2] = 0.;
		vals[3][3] = 1.;
	}

	void translate(float tx, float ty, float tz) {
		vals[0][3] += tx;
		vals[1][3] += ty;
		vals[2][3] += tz;
	}

	void scale(float sx, float sy, float sz) {
		vals[0][0] *= sx;
		vals[1][1] *= sy;
		vals[2][2] *= sz;
	}

	void transpose() {
		float newMat[4][4];
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				newMat[i][j] = vals[j][i];
			}
		}
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				vals[i][j] = newMat[i][j];
			}
		}
	}

	void multiply(Mat4 postMat) {

		float newMat[4][4];
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				float sum = 0.;

				for (int k = 0; k < 4; k++) {

					sum += postMat.vals[i][k] * vals[k][j];

				}

				newMat[i][j] = sum;
			}
		}

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				vals[i][j] = newMat[i][j];
			}
		}
	}

	float* multByVector(float theVec[4]) {

		float result[4] = { 0,0,0,0 };
		for (int i = 0; i < 4; i++) {
			float sum = 0.;
			for (int j = 0; j < 4; j++) {
				sum += vals[i][j] * theVec[j];
			}
			result[i] = sum;
		}

		return result;
	}

};