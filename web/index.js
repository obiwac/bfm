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

class Scenery {
	constructor(model) {
		/** @type: Float32Array */
		this.vbo_data = model.vbo_data

		/** @type: Uint32Array */
		this.indices = model.indices

		this.vao = gl.createVertexArray()
		gl.bindVertexArray(this.vao)

		this.vbo = gl.createBuffer()
		gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo)
		gl.bufferData(gl.ARRAY_BUFFER, this.vbo_data, gl.STATIC_DRAW)

		gl.enableVertexAttribArray(0)
		gl.vertexAttribPointer(0, 3, gl.FLOAT, gl.FALSE, FLOAT32_SIZE * 6, FLOAT32_SIZE * 0)

		this.ibo = gl.createBuffer()
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ibo)
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.indices, gl.STATIC_DRAW)

		gl.enableVertexAttribArray(1)
		gl.vertexAttribPointer(1, 3, gl.FLOAT, gl.FALSE, FLOAT32_SIZE * 6, FLOAT32_SIZE * 3)
	}

	draw() {
		gl.bindVertexArray(this.vao)
		gl.drawElements(gl.TRIANGLES, this.indices.length, gl.UNSIGNED_INT, 0)
	}
}

class Instance {
	constructor(model) {
		/** @type: Uint32Array */
		this.indices = model.indices

		/** @type: Uint32Array */
		this.line_indices = model.line_indices

		/** @type: Float32Array */
		this.coords = model.coords

		/** @type: Float32Array */
		this.effects = model.effects

		/** @type: number */
		this.max_effect = model.max_effect

		this.vao = gl.createVertexArray()
		gl.bindVertexArray(this.vao)

		this.vbo = gl.createBuffer()
		gl.bindBuffer(gl.ARRAY_BUFFER, this.vbo)
		gl.bufferData(gl.ARRAY_BUFFER, this.coords, gl.STATIC_DRAW)

		gl.enableVertexAttribArray(0)
		gl.vertexAttribPointer(0, 3, gl.FLOAT, gl.FALSE, 0, 0)

		this.effects_vbo = gl.createBuffer()
		gl.bindBuffer(gl.ARRAY_BUFFER, this.effects_vbo)
		gl.bufferData(gl.ARRAY_BUFFER, this.effects, gl.STATIC_DRAW)

		gl.enableVertexAttribArray(1)
		gl.vertexAttribPointer(1, 2, gl.FLOAT, gl.FALSE, 0, 0)

		this.ibo = gl.createBuffer()
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ibo)
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.indices, gl.STATIC_DRAW)

		this.line_ibo = gl.createBuffer()
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.line_ibo)
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, this.line_indices, gl.STATIC_DRAW)
	}

	/** @function
	  * @param lines: boolean
	  */
	draw(lines) {
		const location = gl.getUniformLocation(cur_shader.program, "max_effect")
		gl.uniform1f(location, this.max_effect)

		gl.bindVertexArray(this.vao)

		if (lines) {
			gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.line_ibo)
			gl.drawElements(gl.LINES, this.line_indices.length, gl.UNSIGNED_INT, 0)
		}

		else {
			gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.ibo)
			gl.drawElements(gl.TRIANGLES, this.indices.length, gl.UNSIGNED_INT, 0)
		}
	}
}

// rest of the loading

const scenery_shader = new Shader("scenery")
const deformation_shader = new Shader("deformation")
const line_deformation_shader = new Shader("line_deformation")

$SCENERY_LOADING

const mvp_uniform = gl.getUniformLocation(scenery_shader.program, "mvp_matrix")
const fov = TAU / 4

// orbit controls

let recoil = 1.7
let rotation = [-0.425, -0.445]
let origin = [-1.135, -0.955, 0]

let target_recoil = 1.7
let target_rotation = [-0.425, -0.445]
let target_origin = [-1.135, -0.955, 0]

function anim(x, target, multiplier) {
	if (multiplier > 1) {
		return target
	}

	return x + (target - x) * multiplier
}

function anim_vec(x, target, multiplier) {
	let vec = structuredClone(x)

	for (let i = 0; i < x.length; i++) {
		vec[i] = anim(x[i], target[i], multiplier)
	}

	return vec
}

canvas.addEventListener("mousemove", e => {
	if (e.buttons & 0b1) {
		target_rotation[0] += e.movementX / 200
		target_rotation[1] -= e.movementY / 200

		target_rotation[1] = Math.max(-TAU / 4, Math.min(TAU / 4, target_rotation[1]))
	}

	// TODO make panning better

	if (e.buttons & 0b10) {
		target_origin[0] += e.movementX / 200
		target_origin[1] += e.movementY / 200
	}
})

canvas.addEventListener("wheel", e => {
	target_recoil -= e.deltaY / 500
	target_recoil = Math.max(target_recoil, 0.5)
})

// rendering

let prev = 0

/** @function
  * @param {number} now
  */
function render(now) {
	const dt = (now - prev) / 1000

	// update camera parameters

	recoil = anim(recoil, target_recoil, dt * 10)
	rotation = anim_vec(rotation, target_rotation, dt * 20)
	origin = anim_vec(origin, target_origin, dt * 20)

	// clear screen

	gl.clearColor(1, 1, 1, 1)
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT)

	scenery_shader.use()

	// projection stuff

	const proj_mat = new Mat()
	proj_mat.perspective(fov, 0.2, 200)

	const view_mat = new Mat()
	view_mat.translate(0, 0, -Math.pow(recoil, 2))
	view_mat.rotate_2d(...rotation)
	view_mat.translate(...origin)

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
