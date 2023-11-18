// a few constants up top

const TAU = Math.PI * 2
const FLOAT32_SIZE = 4

// initial WebGL setup

const canvas = document.getElementById("canvas")

/** @type {WebGLRenderingContext | WebGL2RenderingContext} */
const gl = canvas.getContext("webgl2") || canvas.getContext("experimental-webgl2") || canvas.getContext("webgl") || canvas.getContext("experimental-webgl")

if (!gl || (!(gl instanceof WebGLRenderingContext) && !(gl instanceof WebGL2RenderingContext))) {
	canvas.hidden = true
	throw Error("Browser doesn't support WebGL")
}

const x_res = gl.drawingBufferWidth
const y_res = gl.drawingBufferHeight

gl.viewport(0, 0, x_res, y_res)
gl.enable(gl.DEPTH_TEST)

// utility classes

class Shader {
	constructor(id) {
		const vert_id = `${id}-vert`
		const frag_id = `${id}-frag`

		const vert_shader = gl.createShader(gl.VERTEX_SHADER)
		const frag_shader = gl.createShader(gl.FRAGMENT_SHADER)

		gl.shaderSource(vert_shader, document.getElementById(vert_id).innerHTML)
		gl.shaderSource(frag_shader, document.getElementById(frag_id).innerHTML)

		gl.compileShader(vert_shader)
		gl.compileShader(frag_shader)

		this.program = gl.createProgram()

		gl.attachShader(this.program, vert_shader)
		gl.attachShader(this.program, frag_shader)

		gl.linkProgram(this.program)

		// MDN detaches the shaders first with 'gl.detachShader'
		// I'm not really sure what purpose this serves

		gl.deleteShader(vert_shader)
		gl.deleteShader(frag_shader)

		if (!gl.getProgramParameter(this.program, gl.LINK_STATUS)) {
			const log = gl.getProgramInfoLog(this.program)
			const vert_compilation_log = gl.getShaderInfoLog(vert_shader);
			const frag_compilation_log = gl.getShaderInfoLog(frag_shader);

			console.error(vert_compilation_log)
			console.error(frag_compilation_log)
			console.error(log)
		}
	}

	use() {
		gl.useProgram(this.program)
	}
}

class Model {
	constructor(model) {
		this.vbo_data = model.vbo_data
		this.indices = model.indices

		this.vbo = gl.createBuffer()
		gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo)
		gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(this.vbo_data), gl.STATIC_DRAW)

		this.ibo = gl.createBuffer()
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ibo)
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, new Uint32Array(this.indices), gl.STATIC_DRAW)
	}

	draw() {
		gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo)
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ibo)

		gl.enableVertexAttribArray(0)
		gl.vertexAttribPointer(0, 3, gl.FLOAT, gl.FALSE, FLOAT32_SIZE * 6, FLOAT32_SIZE * 0)

		gl.enableVertexAttribArray(1)
		gl.vertexAttribPointer(1, 3, gl.FLOAT, gl.FALSE, FLOAT32_SIZE * 6, FLOAT32_SIZE * 3)

		gl.drawElements(gl.TRIANGLES, this.indices.length, gl.UNSIGNED_INT, 0)
	}
}

// rest of the loading

const scenery_shader = new Shader("scenery")
$SCENERY_MODEL_LOADING

const mvp_uniform = gl.getUniformLocation(scenery_shader.program, "mvp_matrix")
const fov = TAU / 6

// rendering

let prev = 0

/** @function
  * @param {number} now
  */
function render(now) {
	const dt = (now - prev) / 1000

	// clear screen

	gl.clearColor(1, 1, 1, 1)
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT)

	scenery_shader.use()

	// projection stuff

	const proj_mat = new Mat()
	proj_mat.perspective(fov, 0.2, 200)

	const view_mat = new Mat()
	view_mat.translate(0, 0, -10)

	const vp_mat = new Mat(view_mat)
	vp_mat.multiply(proj_mat)

	const model_mat = new Mat()
	const mvp_mat = new Mat(model_mat)
	mvp_mat.multiply(vp_mat)

	gl.uniformMatrix4fv(mvp_uniform, false, mvp_mat.data.flat())

	// render scenery

	for (const thing of scenery) {
		thing.draw()
	}

	// continue render loop

	requestAnimationFrame(render)
}

requestAnimationFrame(render)
