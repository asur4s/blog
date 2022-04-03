use std::io;    // prelude 预导入模块
use rand::Rng; // trait 类似接口

fn main() {
    println!("猜数！");
    println!("猜一个数");

    let secret_number = rand::thread_rng().gen_range(1, 101);
    println!("secret number {}", secret_number);

    // let mut foo = 1;
    // let bar = foo;  // immutable
    // foo = 2;

    // 可变变量
    // UTF8，::代表 String 的关联函数。
    let mut guess = String::new();
    // & 表示引用，方法按引用进行传递。
    // &guess 默认不可变，所以必须加入 mut。
    io::stdin().read_line(&mut guess).expect("无法读取行");
    // io::Result: Ok（数据）, Err
    // Result必须处理，否则会被警告。

    println!("你猜测的数是：{}", guess);
}
