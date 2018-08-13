; ModuleID = 'test_complex_struct_passing.c'
source_filename = "test_complex_struct_passing.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.clothes = type { [10 x i8], i32 }
%struct.person_t = type { i32, [10 x i8], %struct.clothes* }

@.str = private unnamed_addr constant [25 x i8] c"%s is wearing red today.\00", align 1
@main.c = private unnamed_addr constant %struct.clothes { [10 x i8] c"red\00\00\00\00\00\00\00", i32 5 }, align 4
@.str.1 = private unnamed_addr constant [10 x i8] c"Jack\00\00\00\00\00\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define void @f(%struct.person_t* %p1) #0 !dbg !7 {
entry:
  %p1.addr = alloca %struct.person_t*, align 8
  %name = alloca i8*, align 8
  store %struct.person_t* %p1, %struct.person_t** %p1.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.person_t** %p1.addr, metadata !28, metadata !DIExpression()), !dbg !29
  call void @llvm.dbg.declare(metadata i8** %name, metadata !30, metadata !DIExpression()), !dbg !32
  %0 = load %struct.person_t*, %struct.person_t** %p1.addr, align 8, !dbg !33
  %name1 = getelementptr inbounds %struct.person_t, %struct.person_t* %0, i32 0, i32 1, !dbg !34
  %arraydecay = getelementptr inbounds [10 x i8], [10 x i8]* %name1, i32 0, i32 0, !dbg !33
  store i8* %arraydecay, i8** %name, align 8, !dbg !32
  %1 = load i8*, i8** %name, align 8, !dbg !35
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([25 x i8], [25 x i8]* @.str, i32 0, i32 0), i8* %1), !dbg !36
  ret void, !dbg !37
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 !dbg !38 {
entry:
  %retval = alloca i32, align 4
  %c = alloca %struct.clothes, align 4
  %p = alloca %struct.person_t, align 8
  %pt = alloca %struct.person_t*, align 8
  store i32 0, i32* %retval, align 4
  call void @llvm.dbg.declare(metadata %struct.clothes* %c, metadata !41, metadata !DIExpression()), !dbg !42
  %0 = bitcast %struct.clothes* %c to i8*, !dbg !42
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 getelementptr inbounds (%struct.clothes, %struct.clothes* @main.c, i32 0, i32 0, i32 0), i64 16, i1 false), !dbg !42
  call void @llvm.dbg.declare(metadata %struct.person_t* %p, metadata !43, metadata !DIExpression()), !dbg !44
  %age = getelementptr inbounds %struct.person_t, %struct.person_t* %p, i32 0, i32 0, !dbg !45
  store i32 10, i32* %age, align 8, !dbg !45
  %name = getelementptr inbounds %struct.person_t, %struct.person_t* %p, i32 0, i32 1, !dbg !45
  %1 = bitcast [10 x i8]* %name to i8*, !dbg !46
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 1 getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i32 0, i32 0), i64 10, i1 false), !dbg !46
  %s = getelementptr inbounds %struct.person_t, %struct.person_t* %p, i32 0, i32 2, !dbg !45
  store %struct.clothes* %c, %struct.clothes** %s, align 8, !dbg !45
  call void @llvm.dbg.declare(metadata %struct.person_t** %pt, metadata !47, metadata !DIExpression()), !dbg !48
  store %struct.person_t* %p, %struct.person_t** %pt, align 8, !dbg !48
  %2 = load %struct.person_t*, %struct.person_t** %pt, align 8, !dbg !49
  call void @f(%struct.person_t* %2), !dbg !50
  ret i32 0, !dbg !51
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test_complex_struct_passing.c", directory: "/home/yongzhe/llvm-versions/llvm-5.0/lib/Analysis/CDG/test/test_debug_info")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 7.0.0 (trunk 323988) (llvm/trunk 323938)"}
!7 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 15, type: !8, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIDerivedType(tag: DW_TAG_typedef, name: "Person", file: !1, line: 12, baseType: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "person_t", file: !1, line: 8, size: 192, elements: !13)
!13 = !{!14, !16, !21}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "age", scope: !12, file: !1, line: 9, baseType: !15, size: 32)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "name", scope: !12, file: !1, line: 10, baseType: !17, size: 80, offset: 32)
!17 = !DICompositeType(tag: DW_TAG_array_type, baseType: !18, size: 80, elements: !19)
!18 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!19 = !{!20}
!20 = !DISubrange(count: 10)
!21 = !DIDerivedType(tag: DW_TAG_member, name: "s", scope: !12, file: !1, line: 11, baseType: !22, size: 64, offset: 128)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!23 = !DIDerivedType(tag: DW_TAG_typedef, name: "Clothes", file: !1, line: 6, baseType: !24)
!24 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "clothes", file: !1, line: 3, size: 128, elements: !25)
!25 = !{!26, !27}
!26 = !DIDerivedType(tag: DW_TAG_member, name: "color", scope: !24, file: !1, line: 4, baseType: !17, size: 80)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "length", scope: !24, file: !1, line: 5, baseType: !15, size: 32, offset: 96)
!28 = !DILocalVariable(name: "p1", arg: 1, scope: !7, file: !1, line: 15, type: !10)
!29 = !DILocation(line: 15, column: 16, scope: !7)
!30 = !DILocalVariable(name: "name", scope: !7, file: !1, line: 16, type: !31)
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!32 = !DILocation(line: 16, column: 11, scope: !7)
!33 = !DILocation(line: 16, column: 18, scope: !7)
!34 = !DILocation(line: 16, column: 22, scope: !7)
!35 = !DILocation(line: 20, column: 40, scope: !7)
!36 = !DILocation(line: 20, column: 5, scope: !7)
!37 = !DILocation(line: 21, column: 1, scope: !7)
!38 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 23, type: !39, isLocal: false, isDefinition: true, scopeLine: 23, isOptimized: false, unit: !0, variables: !2)
!39 = !DISubroutineType(types: !40)
!40 = !{!15}
!41 = !DILocalVariable(name: "c", scope: !38, file: !1, line: 24, type: !23)
!42 = !DILocation(line: 24, column: 13, scope: !38)
!43 = !DILocalVariable(name: "p", scope: !38, file: !1, line: 25, type: !11)
!44 = !DILocation(line: 25, column: 12, scope: !38)
!45 = !DILocation(line: 25, column: 16, scope: !38)
!46 = !DILocation(line: 25, column: 21, scope: !38)
!47 = !DILocalVariable(name: "pt", scope: !38, file: !1, line: 26, type: !10)
!48 = !DILocation(line: 26, column: 13, scope: !38)
!49 = !DILocation(line: 27, column: 7, scope: !38)
!50 = !DILocation(line: 27, column: 5, scope: !38)
!51 = !DILocation(line: 29, column: 5, scope: !38)
