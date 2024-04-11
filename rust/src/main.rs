mod lexer;
use lexer::{Lexer, Token, TokenKind};
use std::{fs, io::Read};

fn usage() {
    let usage = r#"
Usage: compiler-r2 <FILE_NAME>       

Options:
    -h, --help                   Print help
    
    "#;
    println!("{}", usage);
    std::process::exit(1);
}

fn main() -> Result<(), std::io::Error> {
    //  let file_name = "/home/amr/programming/rust/compiler/src/main.rs";
    let mut args = std::env::args();
    let mut file_name = String::new();
    if args.len() > 1 {
        let a1 = args.nth(1).unwrap();
        match a1.as_str() {
            "-h" | "--help" => usage(),
            _ => {
                file_name = a1.to_string();
            }
        }
    } else {
        usage();
    }

    let mut file = fs::File::open(file_name)?;
    let mut src = String::new();
    file.read_to_string(&mut src)?;

    let mut lex = Lexer::new(&src.as_str());

    loop {
        let token: Token = lex.next_token();
        let tok_str = &lex.source[token.start as usize..token.end as usize];
        match token.kind {
            TokenKind::Eof => {
                println!("({}) -> `{}`", token, tok_str);
                break;
            }
            _ => println!("({}) -> `{}`", token, tok_str),
        }
    }
    Ok(())
}
