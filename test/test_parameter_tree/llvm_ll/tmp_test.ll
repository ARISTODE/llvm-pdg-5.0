; ModuleID = 'tmp_test.c'
source_filename = "tmp_test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@a = global i32 4, align 4

; Function Attrs: noinline nounwind optnone uwtable
define void @f(i32* %ptr) #0 {
entry:
  %ptr.addr = alloca i32*, align 8
  store i32* %ptr, i32** %ptr.addr, align 8
  %0 = load i32*, i32** %ptr.addr, align 8
  %1 = load i32, i32* %0, align 4
  %add = add nsw i32 %1, 1
  %2 = load i32*, i32** %ptr.addr, align 8
  store i32 %add, i32* %2, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %b = alloca i32*, align 8
  store i32 0, i32* %retval, align 4
  store i32* @a, i32** %b, align 8
  %0 = load i32*, i32** %b, align 8
  call void @f(i32* %0)
  %1 = load i32*, i32** %b, align 8
  %2 = load i32, i32* %1, align 4
  ret i32 %2
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)"}
