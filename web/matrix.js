class Matrix {
	// matrices are all 4x4, and are initialized as the identity matrix
	// I won't comment on the code here all that much because it's pretty much just computations

	/** @function
	  * @param {Matrix | undefined} template
	  */
	constructor(template) {
		// if we pass a template matrix, copy it
		// otherwise, initialize it to the 4x4 identity matrix

		if (template !== undefined) {
			this.data = JSON.parse(JSON.stringify(template.data)) // I hate javascript 🙂 (yes I know structuredClone exists now but it's too recent)
			return
		}

		this.data = [
			[1.0, 0.0, 0.0, 0.0],
			[0.0, 1.0, 0.0, 0.0],
			[0.0, 0.0, 1.0, 0.0],
			[0.0, 0.0, 0.0, 1.0],
		]
	}

	/** @function
	  * @param {number} left
	  */
	multiply(left) {
		const right = new Matrix(this)

		for (let i = 0; i < 4; i++) {
			for (let j = 0; j < 4; j++) {
				this.data[i][j] =
					left.data[0][j] * right.data[i][0] +
					left.data[1][j] * right.data[i][1] +
					left.data[2][j] * right.data[i][2] +
					left.data[3][j] * right.data[i][3]
			}
		}
	}

	/** @function
	  * @param {number} x
	  * @param {number} y
	  * @param {number} z
	  */
	scale(x, y, z) {
		for (let i = 0; i < 4; i++) {
			this.data[0][i] *= x
			this.data[1][i] *= y
			this.data[2][i] *= z
		}
	}

	/** @function
	  * @param {number} x
	  * @param {number} y
	  * @param {number} z
	  */
	translate(x, y, z) {
		for (let i = 0; i < 4; i++) {
			this.data[3][i] +=
				this.data[0][i] * x +
				this.data[1][i] * y +
				this.data[2][i] * z
		}
	}

	/** @function
	  * @param {number} theta
	  * @param {number} x
	  * @param {number} y
	  * @param {number} z
	  */
	rotate(theta, x, y, z) {
		// theta represents the angle we want to rotate by
		// xyz represents the eigenvector of the matrix transformation of the rotation

		// normalize xyz

		const mag = Math.sqrt(x * x + y * y + z * z)

		x /= -mag
		y /= -mag
		z /= -mag

		const s = Math.sin(theta)
		const c = Math.cos(theta)
		const one_minus_c = 1 - c

		const xx = x * x, yy = y * y, zz = z * z
		const xy = x * y, yz = y * z, zx = z * x
		const xs = x * s, ys = y * s, zs = z * s

		const rotation = new Matrix()

		rotation.data[0][0] = (one_minus_c * xx) + c
		rotation.data[0][1] = (one_minus_c * xy) - zs
		rotation.data[0][2] = (one_minus_c * zx) + ys

		rotation.data[1][0] = (one_minus_c * xy) + zs
		rotation.data[1][1] = (one_minus_c * yy) + c
		rotation.data[1][2] = (one_minus_c * yz) - xs

		rotation.data[2][0] = (one_minus_c * zx) - ys
		rotation.data[2][1] = (one_minus_c * yz) + xs
		rotation.data[2][2] = (one_minus_c * zz) + c

		rotation.data[3][3] = 1

		rotation.multiply(this)
		this.data = rotation.data
	}

	/** @function
	  * @param {number} yaw
	  * @param {number} pitch
	  */
	rotate_2d(yaw, pitch) {
		this.rotate(yaw, 0, 1, 0)
		this.rotate(-pitch, Math.cos(yaw), 0, Math.sin(yaw))
	}

	/** @function
	  * @param {number} fov
	  * @param {number} near
	  * @param {number} far
	  */
	perspective(fov, near, far) {
		const scale = 1 / Math.tan(fov / 2)

		this.data[0][0] = scale
		this.data[1][1] = scale
		this.data[2][2] = -far / (far - near)
		this.data[3][2] = -far * near / (far - near)
		this.data[2][3] = -1
		this.data[3][3] = 0
	}
}
