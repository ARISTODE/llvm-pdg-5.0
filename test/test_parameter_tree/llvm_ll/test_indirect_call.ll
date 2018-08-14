; ModuleID = 'test_indirect_call.c'
source_filename = "test_indirect_call.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.list = type { i32 }

@Global = global i32 10, align 4

; Function Attrs: noinline nounwind optnone uwtable
define void @fun2(i32* %X) #0 {
entry:
  %X.addr = alloca i32*, align 8
  store i32* %X, i32** %X.addr, align 8
  %0 = load i32, i32* @Global, align 4
  %sub = sub nsw i32 %0, 1
  %1 = load i32*, i32** %X.addr, align 8
  %2 = load i32, i32* %1, align 4
  %add = add nsw i32 %2, %sub
  store i32 %add, i32* %1, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define void @fun(i32* %X) #0 {
entry:
  %X.addr = alloca i32*, align 8
  store i32* %X, i32** %X.addr, align 8
  %0 = load i32, i32* @Global, align 4
  %1 = load i32*, i32** %X.addr, align 8
  %2 = load i32, i32* %1, align 4
  %add = add nsw i32 %2, %0
  store i32 %add, i32* %1, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define void @accessF(%struct.list* %L, void (i32*)* %FP) #0 {
entry:
  %L.addr = alloca %struct.list*, align 8
  %FP.addr = alloca void (i32*)*, align 8
  store %struct.list* %L, %struct.list** %L.addr, align 8
  store void (i32*)* %FP, void (i32*)** %FP.addr, align 8
  %0 = load void (i32*)*, void (i32*)** %FP.addr, align 8
  %1 = load %struct.list*, %struct.list** %L.addr, align 8
  %Data = getelementptr inbounds %struct.list, %struct.list* %1, i32 0, i32 0
  call void %0(i32* %Data)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define void @passF(%struct.list* %L) #0 {
entry:
  %L.addr = alloca %struct.list*, align 8
  store %struct.list* %L, %struct.list** %L.addr, align 8
  %0 = load %struct.list*, %struct.list** %L.addr, align 8
  call void @accessF(%struct.list* %0, void (i32*)* @fun)
  %1 = load %struct.list*, %struct.list** %L.addr, align 8
  call void @accessF(%struct.list* %1, void (i32*)* @fun2)
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 {
entry:
  %X = alloca %struct.list*, align 8
  %Y = alloca %struct.list*, align 8
  %call = call noalias i8* @malloc(i64 4) #2
  %0 = bitcast i8* %call to %struct.list*
  store %struct.list* %0, %struct.list** %X, align 8
  %1 = alloca i8, i64 4, align 16
  %2 = bitcast i8* %1 to %struct.list*
  store %struct.list* %2, %struct.list** %Y, align 8
  %3 = load %struct.list*, %struct.list** %X, align 8
  call void @passF(%struct.list* %3)
  ret i32 0
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)"}
