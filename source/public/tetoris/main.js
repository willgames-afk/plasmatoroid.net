let tetrs = {
	i: 0b00000000,
	t: 0b00010101,
	l: 0b00011001,
	j: 0b00010011,
	s: 0b00001101,
	z: 0b00010110,
	o: 0b00011100
}

let cols = {
	1: "#000",
	2: "#FFF",
	3: "#888"
};
cols[tetrs.i] = "#0FF"
cols[tetrs.t] = "#80F"
cols[tetrs.l] = "#F80"
cols[tetrs.j] = "#00F"
cols[tetrs.s] = "#0F0"
cols[tetrs.z] = "#F00"
cols[tetrs.o] = "#FF0"

let config = {
	pieceRandomizer: "bag", //or "random" or "nes"
	playfieldWidth: 10,
	playfieldHeight: 40,
	playfieldTop: 21,
	peekTop: true,
	useSRS: true,
	showStats: false,
	hold: true,
	nextLen: 1, // how many upcoming tetrominos to display (0 for none)
	softDropFactor: 20, // multiplied by normal drop factor
	showGhost: true, //show where piece will go if hard dropped
	lockdown: "extended", //or "infinite" or "classic"
	// todo: lockdown 
	// todo: levels and scoring
}

let dropTime = level => Math.pow(0.8 - ((level-1) * 0.007),level-1);

let game = {
	bag: [tetrs.i,tetrs.t,tetrs.l,tetrs.j,tetrs.s,tetrs.z,tetrs.o],
	bagID: -1,
	playfield: new Array(config.playfieldHeight * config.playfieldWidth).fill(1),
	pieceX: 0,
	pieceY: 0,
	pieceR: 0,
	pieceMino: -1,
	dropCounter: 0,
	lockCounter: 0.5,
	mode: 0,
	droprate: 0.5,
}

function nextPiece() {
	game.bagID--;
	if (game.bagID < 0) {
		//fisher-yates shuffle
		let j,t;
		for (let i=game.bag.length-1;i>=1;i--) {
			j = Math.floor(Math.random() * (i+1));
			t = game.bag[i];
			game.bag[i] = game.bag[j];
			game.bag[j] = t;
		}
		game.bagID = game.bag.length-1;
	}
	return game.bag[game.bagID];
}

/*
Tetris SRS (super rotation system) uses offsets to shove tetrominos around while
rotating, in hopes of allowing you to avoid rotating into a wall.

order
tetromino
	starting orientation
		trial
			point

To calculate the shift for the piece
- get your chosen offset in starting orientation and subtract that from the position
- get your chosen offset in the new orientation and add that to the position

*/
let srsOffsets = {
	i: [[[-1,0],[2,0],[-1,0],[2,0]],[[0,0],[0,0],[0,2],[0,-1]],[[2,0],[-1,0],[2,1],[-1,1]],[[1,0],[1,0],[1,-1],[1,2]]],
	other: [[[0,0],[0,0],[0,0],[0,0]],[[-1,0],[-1,1],[0,-2],[-1,-2]],[[0,0],[0,0],[0,0],[0,0]],[[1,0],[1,1],[0,-2],[1,-2]]],
}

const c = document.getElementById("main");
c.width = 256;
c.height = 256;
const ctx = c.getContext("2d");

function getPF(x,y) { //get playfield index
	if (x < 0) return -1;
	if (x >= config.playfieldWidth) return -1;
	return x + y * config.playfieldWidth;
}

// rotation is 90 degrees counter clockwise
function drawTetr(x, y, tetromino, r, clear, test) {
	let v0 = -1;
	let v1 = -1;
	let index = 0;
	let color = clear ? 1 : tetromino;

	// i piece rotation is weird
	if (tetromino == tetrs.i) {
		v1 = ((r & 2)) >> 1;
		if (r & 1) {
			index = v1;
			v1 = v0;
			v0 = index;
		}
		//console.log(v);
		for (let i=0;i<4;i++) {
			index = getPF(x + v0, y + v1);
			if (test) {
				if (game.playfield[index] != 1) return false;
			} else {
				game.playfield[index] = color;
			}
			//ctx.fillRect((x + v[0]) * 10,(y + v[1]) * 10, 10, 10);
			if (r & 1) {
				v1++;
			} else {
				v0++;
			}
		}
		return true;
	}

	index = getPF(x, y);
	if (test) {
		if (game.playfield[index] != 1) return false;
	} else {
		game.playfield[index] = color;
	}
	//for other tetrominos, rotate by rotating its bits
	if (tetromino != tetrs.o) {
		tetromino = tetromino << (r * 2);
		tetromino |= (tetromino & 0xFF00) >> 8;
	}

	for (let i=0;i<8;i++) {
		index = i&4 ? -1 : 1;
		if ((i&2)>>1) {
			v1 += index;
		} else {
			v0 += index;
		}
		if (tetromino >> i & 1) {
			index = getPF(x + v1, y + v0);
			//console.log((x+ v[1]), (y+ v[0]) )
			if (test) {
				if (game.playfield[index] != 1) return false;
			} else {
				game.playfield[index] = color;
			}
	
		}
	}
	return true
}

function clearLines() {
	let c = 0;
	for (let i=0;i<game.playfield.length;i++) {
		if (game.playfield[i] == 1) {
			i += config.playfieldWidth;
			i -= i % config.playfieldWidth;
			c = 0;
		} else {
			c++; //ewww c++
			console.log(c);
			if (c == config.playfieldWidth) {
				c = 0;
				console.log("CLEARING AT i", i);
				game.playfield.splice(i - config.playfieldWidth + 1, config.playfieldWidth);
				//alert("LINE!");
			}
		}
	}
}

function drawPlayfield() {
	ctx.fillRect(0,0,c.clientWidth,c.clientHeight);
	const offsX = 70;
	const offsY = 40;
	let x = 0,y = 0;
	for (let i=0;i<game.playfield.length;i++) {
		ctx.fillStyle = cols[game.playfield[i]];
		ctx.fillRect(x++ * 10 + offsX,y * 10 + offsY + (config.playfieldTop - 1) * 10, 10,10);
		if (x >= config.playfieldWidth) {
			x = 0;
			y--;
		}
	}
	ctx.strokeStyle = cols[3];
	ctx.lineWidth = 2;
	ctx.strokeRect(offsX, offsY, config.playfieldWidth * 10, config.playfieldTop * 10);
}
let prev = performance.now() - 16.66
let r = 0;
function loop(now) {
	let dt = (now-prev) / 1000;
	prev = now;
	requestAnimationFrame(loop);
	switch(game.mode) {
	case 0:
		//generate new mino
		game.pieceMino = nextPiece();
		game.pieceR = 0;
		game.pieceX = 4;
		game.pieceY = 21;
		if (!drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, false, true)) {
			game.mode = 3;
			return;
		}
		if (drawTetr(game.pieceX,game.pieceY-1, game.pieceMino , game.pieceR, false, true)) game.pieceY--;

		drawTetr(game.pieceX,game.pieceY, game.pieceMino , game.pieceR);
		drawPlayfield();
		game.dropCounter = game.droprate; //todo: change based on level
		game.lockCounter = 0.5;	
		game.mode++;
		break;
	case 1:
		//drop
		game.dropCounter -= dt * (game.softDrop ? 20 : 1);
		if (game.dropCounter > 0) break;
		drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, true, false); // clear current tetromino
		game.pieceY--;
		if (!drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, false, true)) {
			game.pieceY++;
			game.lockCounter = 0.5;	
			game.mode++;
		} else {
			game.dropCounter = game.droprate;
		}
		drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, false, false);
		drawPlayfield();
		break;
	case 2:
		//lock
		game.lockCounter -= dt;
		if (drawTetr(game.pieceX,game.pieceY-1, game.pieceMino, game.pieceR, false, true)) {
			game.dropCounter = game.droprate;
			game.mode = 1;
			break;
		}
		if (game.lockCounter > 0) break;
		game.mode = 0;
		clearLines();
		break;
	case 3:
		console.log("Gameover!");
		break;
	}
}
addEventListener("keydown",(e)=>{
	if (game.mode == 3) return; //game over
	if (game.mode == 0) return; //last piece is locked but next one hasn't generated yet
	drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, true, false)
	switch(e.key) {
	case "ArrowLeft":
	case "a":
	case "A":
		game.pieceX--;
		if (!drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, false, true)) {
			game.pieceX++;
		}
		break;
	case "ArrowRight":
	case "d":
	case "D":
		game.pieceX++;
		if (!drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, false, true)) {
			game.pieceX--;
		}
		break;
	case "ArrowDown":
	case "s":
	case "S":
		game.softDrop = true;
		break;
	case "ArrowUp":
	case "x":
	case "X":
	case "w":
	case "W":
		game.pieceR = (game.pieceR + 1) % 4
		if (!drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, false, true)) {
			game.pieceR = (game.pieceR + 3) % 4;
		}
		break;
	case "z":
	case "Z":
	case "Control":
		game.pieceR = (game.pieceR + 3) % 4;
		if (!drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR, false, true)) {
			game.pieceR = (game.pieceR + 1) % 4
		}
		break;
	}
	drawTetr(game.pieceX,game.pieceY, game.pieceMino, game.pieceR)
	drawPlayfield();
	//console.log(e.key);
})
addEventListener("keyup",(e)=>{
	switch(e.key) {
	case "ArrowDown":
	case "s":
	case "S":
		game.softDrop = false;
	}
})
loop(performance.now());