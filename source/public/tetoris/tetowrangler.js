// Loading mmd synthv teto

// shaders:
const vs = `
	attribute vec3 vpos;
	void main() {
		gl_Position = vec4(vpos.x, vpos.y, vpos.z, 0) * 10.0;
	}
`
const fs = `
	void main() {
		gl_FragColor = vec4(1.0,1.0,0.0,1.0);
	}
`

function loadGPUCode(gl) {
	const prog = gl.createProgram();
	const vshader = gl.createShader(gl.VERTEX_SHADER);
	gl.shaderSource(vshader, vs);
	gl.compileShader(vshader);
	gl.attachShader(prog, vshader);
	const fshader = gl.createShader(gl.FRAGMENT_SHADER);
	gl.shaderSource(fshader,fs);
	gl.compileShader(fshader);
	gl.attachShader(prog, fshader);
	
	gl.linkProgram(prog);
	return prog;
	let locs = {};
	locs.vpos = gl.getAttribLocation(prog, "vpos");
}

Promise.all([
	new Promise(res=>{
		addEventListener("load",()=>{
			res();

		})
	}),
	fetch("./teto.pmx")
	.then(httpres=>{
		if (!httpres.ok) console.warn(`Teto is not ok (${httpres.status})`);
		return httpres.blob();
	})
	.then(blob=>blob.arrayBuffer())
])
.then(([_,buf])=>{
	let d = new DataView(buf);
	let i=0;
	i += 8;
	let globals = d.getUint8(i); i++;
	console.log(`${globals} g`);
	let xtraVecs = 0;
	let vertIndType = 4;
	let boneIndType = 4;
	let stringType = 'utf-8';
	if (globals > 0) stringType = d.getUint8(i) ? 'utf-8' : 'utf-16le';
	if (globals > 1) xtraVecs = d.getUint8(i+1);
	if (globals > 2) vertIndType = d.getUint8(i+2);
	if (globals > 5) boneIndType = d.getUint8(i+5);
	i += globals;
	let tdec = new TextDecoder(stringType);

	i = skipstr(d,i,tdec);
	i = skipstr(d,i,tdec);
	i = skipstr(d,i,tdec);
	i = skipstr(d,i,tdec);
	let vertCount = d.getInt32(i, true); i += 4;
	console.log(`${vertCount} v`);

	let vpos = [];
	const skiptable = [boneIndType, boneIndType * 2 + 4, boneIndType * 4 + 16, boneIndType * 2 + 4 + 12 * 3, boneIndType * 4 + 16];
	for (let v=0;v<vertCount;v++) {
		vpos.push(d.getFloat32(i,true));
		vpos.push(d.getFloat32(i+4,true));
		vpos.push(d.getFloat32(i+8,true));
		i += 12 + 12 + 8 + 16 * xtraVecs;
		const kind = d.getUint8(i);
		i += 1 + skiptable[kind] + 4;
	}
	vpos = new Float32Array(vpos);
	console.log(`parsed vertex data!!`);
	console.log(vpos);

	let triCount = d.getInt32(i,true); i += 4;
	console.log(`${triCount/3} t`);
	let triBuffer = d.buffer.slice(i, i + vertIndType * triCount);
	let tris;
	switch(vertIndType) {
	case 1: tris = new Uint8Array(triBuffer);
	case 2: tris = new Uint16Array(triBuffer);
	case 4: tris = new Uint32Array(triBuffer); //NOTE: this should technically be Int32 and not Uint32, but webgl doesn't support signed indices
	}
	i += vertIndType * triCount;
	console.log(`parsed triangle indices!!`);
	console.log(tris);

	let canv = document.getElementById("teto");
	let gl = canv.getContext("webgl");
		const gltype = {
		1: gl.UNSIGNED_BYTE,
		2: gl.UNSIGNED_SHORT,
		4: gl.UNSIGNED_INT
	}

	if (vertIndType == 4) {
		let ext = gl.getExtension("OES_element_index_uint");
		if (ext === null) {
			console.log("err: uint index not supported! rendering will fail :(");
			//todo: convert to multiple 16-bit-indexed meshes
		}
	}

	//vpos = new Float32Array([0, 0, 1, 100, 0, 1, 100, 100, 1, 0, 100, 1]);
	//tris = new Uint32Array([0, 1, 2, 1, 2, 3]);

	const prog = loadGPUCode(gl);
	const gpu_vpos = gl.createBuffer();
	gl.bindBuffer(gl.ARRAY_BUFFER, gpu_vpos);
	gl.bufferData(gl.ARRAY_BUFFER, vpos, gl.STATIC_DRAW);
	const gpu_vposP = gl.getAttribLocation(prog, "vpos")
	gl.enableVertexAttribArray(gpu_vposP);
	gl.vertexAttribPointer(gpu_vposP,3, gl.FLOAT,false,12,0);
	const gpu_tris = gl.createBuffer();
	gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, gpu_tris);
	gl.bufferData(gl.ELEMENT_ARRAY_BUFFER,tris, gl.STATIC_DRAW);
	gl.useProgram(prog);

	gl.viewport(0,0,gl.width,gl.height);



	gl.clearColor(0,0,0,1);
	gl.clearDepth(1);
	gl.enable(gl.DEPTH_TEST);
	gl.depthFunc(gl.LEQUAL);
	gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

	//gl.drawArrays(gl.LINES,0,tris.length);
	gl.drawElements(gl.TRIANGLES, tris.length, gltype[vertIndType], 0);
	console.log("rendered?")
})

function skipstr(d, i, tdec) {
	let strlen = d.getInt32(i,true);
	if (strlen <= 0) return i + 4;
	let strbuf = d.buffer.slice(i + 4, i + 4 + strlen);
	console.log(tdec.decode(strbuf));
	i += strlen + 4;
	return i;
}

/*
pmx header
4 signature
4 version (float)
1+n globals, n is usually 8
4+n model name
4+n model universal name 
4+n comments
4+n universal comments
...followed by pmx body
4+n vertices
4+n "surfaces" (triangles)
4+n textures
4+n materials
4+n bones
4+n morphs
4+n displayframes
4+n rigidbodies
4+n joints
4+n softbodies

Globals:
index size desc
0 1 text encoding, 0= utf-16LE, 1 = utf-8
1 1 additional vec4 count, 0-4
2 1 vertex index size, 1, 2, or 4. 
3 1 texture index size
4 1 material index size
5 1 bone index size
6 1 morph index size
7 1 rigidbody index size


*/