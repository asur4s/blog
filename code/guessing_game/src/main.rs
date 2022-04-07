use rand::Rng; // trait 类似接口
use std::cmp::Ordering;
use std::io; // prelude 预导入模块

fn main() {
    println!("猜数！");
    let secret_number = rand::thread_rng().gen_range(1, 101);
    println!("secret number {}", secret_number);

    loop {
        println!("猜一个数");
        // UTF8，::代表 String 的关联函数。
        let mut guess = String::new();
        // io::Result: Ok（数据）, Err。Result必须处理，否则会被警告。
        io::stdin().read_line(&mut guess).expect("无法读取行");
        // 类型转换，parse 可以解析成整数。优雅的异常处理
        let guess: u32 = match guess.trim().parse(){
            Ok(num) => num,
            Err(_) => continue,
        };

        println!("你猜测的数是：{}", guess);
        match guess.cmp(&secret_number) {
            Ordering::Less => println!("Too small!"),
            Ordering::Greater => println!("Too big!"),
            Ordering::Equal => {
                println!("You win!");
                break;
            },
        }
    }
}
