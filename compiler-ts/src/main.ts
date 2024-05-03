import * as lexer from "./lexer";
import * as fs from 'fs';

let tokens = lexer.tokenize("Name Age 454 ;");

const file = fs.readFileSync('src/main.js', 'utf-8');
console.log(file);
for(let i = 0; i < tokens.length; i++){
	console.log(tokens[i].type + " => " + tokens[i].value);
}
