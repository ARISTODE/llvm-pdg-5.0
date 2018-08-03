; ModuleID = 'test_debug_presence.c'
source_filename = "test_debug_presence.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.person_t = type { i32, [10 x i8] }

@main.p = private unnamed_addr constant %struct.person_t { i32 10, [10 x i8] c"Jack\00\00\00\00\00\00" }, align 4

; Function Attrs: noinline nounwind optnone uwtable
define i32 @f(%struct.person_t* %p1) #0 !dbg !7 {
entry:
  %p1.addr = alloca %struct.person_t*, align 8
  store %struct.person_t* %p1, %struct.person_t** %p1.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.person_t** %p1.addr, metadata !21, metadata !DIExpression()), !dbg !22
  ret i32 10, !dbg !23
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 !dbg !24 {
entry:
  %p = alloca %struct.person_t, align 4
  %p1 = alloca %struct.person_t*, align 8
  call void @llvm.dbg.declare(metadata %struct.person_t* %p, metadata !27, metadata !DIExpression()), !dbg !28
  %0 = bitcast %struct.person_t* %p to i8*, !dbg !28
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 bitcast (%struct.person_t* @main.p to i8*), i64 16, i1 false), !dbg !28
  call void @llvm.dbg.declare(metadata %struct.person_t** %p1, metadata !29, metadata !DIExpression()), !dbg !30
  store %struct.person_t* %p, %struct.person_t** %p1, align 8, !dbg !30
  %1 = load %struct.person_t*, %struct.person_t** %p1, align 8, !dbg !31
  %call = call i32 @f(%struct.person_t* %1), !dbg !32
  ret i32 0, !dbg !33
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test_debug_presence.c", directory: "/home/yongzhe/llvm-versions/llvm-5.0/lib/Analysis/CDG/test/test_debug_info")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)"}
!7 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 9, type: !8, isLocal: false, isDefinition: true, scopeLine: 9, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_typedef, name: "Person", file: !1, line: 7, baseType: !13)
!13 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "person_t", file: !1, line: 4, size: 128, elements: !14)
!14 = !{!15, !16}
!15 = !DIDerivedType(tag: DW_TAG_member, name: "age", scope: !13, file: !1, line: 5, baseType: !10, size: 32)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !13, file: !1, line: 6, baseType: !17, size: 80, offset: 32)
!17 = !DICompositeType(tag: DW_TAG_array_type, baseType: !18, size: 80, elements: !19)
!18 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!19 = !{!20}
!20 = !DISubrange(count: 10)
!21 = !DILocalVariable(name: "p1", arg: 1, scope: !7, file: !1, line: 9, type: !11)
!22 = !DILocation(line: 9, column: 15, scope: !7)
!23 = !DILocation(line: 10, column: 5, scope: !7)
!24 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 13, type: !25, isLocal: false, isDefinition: true, scopeLine: 13, isOptimized: false, unit: !0, variables: !2)
!25 = !DISubroutineType(types: !26)
!26 = !{!10}
!27 = !DILocalVariable(name: "p", scope: !24, file: !1, line: 14, type: !12)
!28 = !DILocation(line: 14, column: 12, scope: !24)
!29 = !DILocalVariable(name: "p1", scope: !24, file: !1, line: 15, type: !11)
!30 = !DILocation(line: 15, column: 13, scope: !24)
!31 = !DILocation(line: 16, column: 7, scope: !24)
!32 = !DILocation(line: 16, column: 5, scope: !24)
!33 = !DILocation(line: 17, column: 1, scope: !24)
