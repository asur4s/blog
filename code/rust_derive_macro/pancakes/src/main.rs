use hello_macro::HelloMacro;
use hello_macro_derive::MacroName;

// 标注 Macro 名称
#[derive(MacroName)]
struct Pancakes;

fn main() {
    Pancakes::hello_macro();
}
