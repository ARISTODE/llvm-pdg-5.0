; ModuleID = 'test_multiuse.c'
source_filename = "test_multiuse.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"%d + %f\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define void @use1(i32* %addr) #0 !dbg !10 {
entry:
  %addr.addr = alloca i32*, align 8
  store i32* %addr, i32** %addr.addr, align 8
  call void @llvm.dbg.declare(metadata i32** %addr.addr, metadata !13, metadata !DIExpression()), !dbg !14
  %0 = load i32*, i32** %addr.addr, align 8, !dbg !15
  %1 = load i32, i32* %0, align 4, !dbg !16
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %1), !dbg !17
  ret void, !dbg !18
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define void @use2(i32* %addr) #0 !dbg !19 {
entry:
  %addr.addr = alloca i32*, align 8
  store i32* %addr, i32** %addr.addr, align 8
  call void @llvm.dbg.declare(metadata i32** %addr.addr, metadata !20, metadata !DIExpression()), !dbg !21
  %0 = load i32*, i32** %addr.addr, align 8, !dbg !22
  %arrayidx = getelementptr inbounds i32, i32* %0, i64 1, !dbg !22
  %1 = load i32, i32* %arrayidx, align 4, !dbg !22
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %1), !dbg !23
  ret void, !dbg !24
}

; Function Attrs: noinline nounwind optnone uwtable
define void @multiuse(i32* %a1, i32* %a2) #0 !dbg !25 {
entry:
  %a1.addr = alloca i32*, align 8
  %a2.addr = alloca i32*, align 8
  store i32* %a1, i32** %a1.addr, align 8
  call void @llvm.dbg.declare(metadata i32** %a1.addr, metadata !28, metadata !DIExpression()), !dbg !29
  store i32* %a2, i32** %a2.addr, align 8
  call void @llvm.dbg.declare(metadata i32** %a2.addr, metadata !30, metadata !DIExpression()), !dbg !31
  %0 = load i32*, i32** %a1.addr, align 8, !dbg !32
  call void @use1(i32* %0), !dbg !33
  %1 = load i32*, i32** %a2.addr, align 8, !dbg !34
  call void @use2(i32* %1), !dbg !35
  ret void, !dbg !36
}

; Function Attrs: noinline nounwind optnone uwtable
define void @f(i32 %a, float %b) #0 !dbg !37 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca float, align 4
  store i32 %a, i32* %a.addr, align 4
  call void @llvm.dbg.declare(metadata i32* %a.addr, metadata !41, metadata !DIExpression()), !dbg !42
  store float %b, float* %b.addr, align 4
  call void @llvm.dbg.declare(metadata float* %b.addr, metadata !43, metadata !DIExpression()), !dbg !44
  %0 = load i32, i32* %a.addr, align 4, !dbg !45
  %1 = load float, float* %b.addr, align 4, !dbg !46
  %conv = fpext float %1 to double, !dbg !46
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %0, double %conv), !dbg !47
  ret void, !dbg !48
}

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 !dbg !49 {
entry:
  %heap = alloca i32*, align 8
  %local1 = alloca i32, align 4
  %local2 = alloca i32, align 4
  %flt = alloca float, align 4
  call void @llvm.dbg.declare(metadata i32** %heap, metadata !52, metadata !DIExpression()), !dbg !53
  call void @llvm.dbg.declare(metadata i32* %local1, metadata !54, metadata !DIExpression()), !dbg !55
  store i32 7, i32* %local1, align 4, !dbg !55
  call void @llvm.dbg.declare(metadata i32* %local2, metadata !56, metadata !DIExpression()), !dbg !57
  store i32 8, i32* %local2, align 4, !dbg !57
  call void @llvm.dbg.declare(metadata float* %flt, metadata !58, metadata !DIExpression()), !dbg !59
  store float 0x401154FE00000000, float* %flt, align 4, !dbg !59
  %0 = load i32, i32* %local1, align 4, !dbg !60
  %1 = load float, float* %flt, align 4, !dbg !61
  call void @f(i32 %0, float %1), !dbg !62
  %call = call noalias i8* @malloc(i64 16) #4, !dbg !63
  %2 = bitcast i8* %call to i32*, !dbg !64
  store i32* %2, i32** %heap, align 8, !dbg !65
  call void @multiuse(i32* %local2, i32* %local1), !dbg !66
  %3 = load i32*, i32** %heap, align 8, !dbg !67
  call void @multiuse(i32* %3, i32* %local1), !dbg !68
  ret i32 0, !dbg !69
}

; Function Attrs: nounwind
declare noalias i8* @malloc(i64) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3)
!1 = !DIFile(filename: "test_multiuse.c", directory: "/home/yongzhe/llvm-versions/llvm-5.0/lib/Analysis/CDG/test/test_debug_info")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!5 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{!"clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)"}
!10 = distinct !DISubprogram(name: "use1", scope: !1, file: !1, line: 4, type: !11, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!11 = !DISubroutineType(types: !12)
!12 = !{null, !4}
!13 = !DILocalVariable(name: "addr", arg: 1, scope: !10, file: !1, line: 4, type: !4)
!14 = !DILocation(line: 4, column: 16, scope: !10)
!15 = !DILocation(line: 5, column: 18, scope: !10)
!16 = !DILocation(line: 5, column: 17, scope: !10)
!17 = !DILocation(line: 5, column: 2, scope: !10)
!18 = !DILocation(line: 6, column: 1, scope: !10)
!19 = distinct !DISubprogram(name: "use2", scope: !1, file: !1, line: 8, type: !11, isLocal: false, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!20 = !DILocalVariable(name: "addr", arg: 1, scope: !19, file: !1, line: 8, type: !4)
!21 = !DILocation(line: 8, column: 16, scope: !19)
!22 = !DILocation(line: 9, column: 17, scope: !19)
!23 = !DILocation(line: 9, column: 2, scope: !19)
!24 = !DILocation(line: 10, column: 1, scope: !19)
!25 = distinct !DISubprogram(name: "multiuse", scope: !1, file: !1, line: 12, type: !26, isLocal: false, isDefinition: true, scopeLine: 12, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!26 = !DISubroutineType(types: !27)
!27 = !{null, !4, !4}
!28 = !DILocalVariable(name: "a1", arg: 1, scope: !25, file: !1, line: 12, type: !4)
!29 = !DILocation(line: 12, column: 20, scope: !25)
!30 = !DILocalVariable(name: "a2", arg: 2, scope: !25, file: !1, line: 12, type: !4)
!31 = !DILocation(line: 12, column: 29, scope: !25)
!32 = !DILocation(line: 13, column: 10, scope: !25)
!33 = !DILocation(line: 13, column: 5, scope: !25)
!34 = !DILocation(line: 14, column: 10, scope: !25)
!35 = !DILocation(line: 14, column: 5, scope: !25)
!36 = !DILocation(line: 15, column: 1, scope: !25)
!37 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 17, type: !38, isLocal: false, isDefinition: true, scopeLine: 17, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!38 = !DISubroutineType(types: !39)
!39 = !{null, !5, !40}
!40 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!41 = !DILocalVariable(name: "a", arg: 1, scope: !37, file: !1, line: 17, type: !5)
!42 = !DILocation(line: 17, column: 12, scope: !37)
!43 = !DILocalVariable(name: "b", arg: 2, scope: !37, file: !1, line: 17, type: !40)
!44 = !DILocation(line: 17, column: 21, scope: !37)
!45 = !DILocation(line: 18, column: 23, scope: !37)
!46 = !DILocation(line: 18, column: 26, scope: !37)
!47 = !DILocation(line: 18, column: 5, scope: !37)
!48 = !DILocation(line: 19, column: 1, scope: !37)
!49 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 21, type: !50, isLocal: false, isDefinition: true, scopeLine: 21, isOptimized: false, unit: !0, variables: !2)
!50 = !DISubroutineType(types: !51)
!51 = !{!5}
!52 = !DILocalVariable(name: "heap", scope: !49, file: !1, line: 22, type: !4)
!53 = !DILocation(line: 22, column: 7, scope: !49)
!54 = !DILocalVariable(name: "local1", scope: !49, file: !1, line: 23, type: !5)
!55 = !DILocation(line: 23, column: 6, scope: !49)
!56 = !DILocalVariable(name: "local2", scope: !49, file: !1, line: 24, type: !5)
!57 = !DILocation(line: 24, column: 6, scope: !49)
!58 = !DILocalVariable(name: "flt", scope: !49, file: !1, line: 25, type: !40)
!59 = !DILocation(line: 25, column: 11, scope: !49)
!60 = !DILocation(line: 28, column: 7, scope: !49)
!61 = !DILocation(line: 28, column: 15, scope: !49)
!62 = !DILocation(line: 28, column: 5, scope: !49)
!63 = !DILocation(line: 29, column: 15, scope: !49)
!64 = !DILocation(line: 29, column: 9, scope: !49)
!65 = !DILocation(line: 29, column: 7, scope: !49)
!66 = !DILocation(line: 30, column: 2, scope: !49)
!67 = !DILocation(line: 31, column: 11, scope: !49)
!68 = !DILocation(line: 31, column: 2, scope: !49)
!69 = !DILocation(line: 32, column: 1, scope: !49)
