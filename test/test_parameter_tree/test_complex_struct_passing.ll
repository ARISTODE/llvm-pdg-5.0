; ModuleID = 'test_complex_struct_passing.c'
source_filename = "test_complex_struct_passing.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.clothes = type { [10 x i8], i32 }
%struct.person_t = type { i32, [10 x i8], %struct.clothes* }

@.str = private unnamed_addr constant [24 x i8] c"%s is wearing %s today.\00", align 1
@main.c = private unnamed_addr constant %struct.clothes { [10 x i8] c"red\00\00\00\00\00\00\00", i32 5 }, align 4
@.str.1 = private unnamed_addr constant [10 x i8] c"Jack\00\00\00\00\00\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define void @f(%struct.person_t* %p1) #0 !dbg !7 {
entry:
  %p1.addr = alloca %struct.person_t*, align 8
  %name = alloca i8*, align 8
  %color = alloca i8*, align 8
  store %struct.person_t* %p1, %struct.person_t** %p1.addr, align 8
  call void @llvm.dbg.declare(metadata %struct.person_t** %p1.addr, metadata !28, metadata !DIExpression()), !dbg !29
  call void @llvm.dbg.declare(metadata i8** %name, metadata !30, metadata !DIExpression()), !dbg !32
  %0 = load %struct.person_t*, %struct.person_t** %p1.addr, align 8, !dbg !33
  %name1 = getelementptr inbounds %struct.person_t, %struct.person_t* %0, i32 0, i32 1, !dbg !34
  %arraydecay = getelementptr inbounds [10 x i8], [10 x i8]* %name1, i32 0, i32 0, !dbg !33
  store i8* %arraydecay, i8** %name, align 8, !dbg !32
  call void @llvm.dbg.declare(metadata i8** %color, metadata !35, metadata !DIExpression()), !dbg !36
  %1 = load %struct.person_t*, %struct.person_t** %p1.addr, align 8, !dbg !37
  %s = getelementptr inbounds %struct.person_t, %struct.person_t* %1, i32 0, i32 2, !dbg !38
  %2 = load %struct.clothes*, %struct.clothes** %s, align 8, !dbg !38
  %color2 = getelementptr inbounds %struct.clothes, %struct.clothes* %2, i32 0, i32 0, !dbg !39
  %arraydecay3 = getelementptr inbounds [10 x i8], [10 x i8]* %color2, i32 0, i32 0, !dbg !37
  store i8* %arraydecay3, i8** %color, align 8, !dbg !36
  %3 = load i8*, i8** %name, align 8, !dbg !40
  %4 = load i8*, i8** %color, align 8, !dbg !41
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([24 x i8], [24 x i8]* @.str, i32 0, i32 0), i8* %3, i8* %4), !dbg !42
  ret void, !dbg !43
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i32 @printf(i8*, ...) #2

; Function Attrs: noinline nounwind optnone uwtable
define i32 @main() #0 !dbg !44 {
entry:
  %retval = alloca i32, align 4
  %c = alloca %struct.clothes, align 4
  %p = alloca %struct.person_t, align 8
  %pt = alloca %struct.person_t*, align 8
  store i32 0, i32* %retval, align 4
  call void @llvm.dbg.declare(metadata %struct.clothes* %c, metadata !47, metadata !DIExpression()), !dbg !48
  %0 = bitcast %struct.clothes* %c to i8*, !dbg !48
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 getelementptr inbounds (%struct.clothes, %struct.clothes* @main.c, i32 0, i32 0, i32 0), i64 16, i1 false), !dbg !48
  call void @llvm.dbg.declare(metadata %struct.person_t* %p, metadata !49, metadata !DIExpression()), !dbg !50
  %age = getelementptr inbounds %struct.person_t, %struct.person_t* %p, i32 0, i32 0, !dbg !51
  store i32 10, i32* %age, align 8, !dbg !51
  %name = getelementptr inbounds %struct.person_t, %struct.person_t* %p, i32 0, i32 1, !dbg !51
  %1 = bitcast [10 x i8]* %name to i8*, !dbg !52
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 1 getelementptr inbounds ([10 x i8], [10 x i8]* @.str.1, i32 0, i32 0), i64 10, i1 false), !dbg !52
  %s = getelementptr inbounds %struct.person_t, %struct.person_t* %p, i32 0, i32 2, !dbg !51
  store %struct.clothes* %c, %struct.clothes** %s, align 8, !dbg !51
  call void @llvm.dbg.declare(metadata %struct.person_t** %pt, metadata !53, metadata !DIExpression()), !dbg !54
  store %struct.person_t* %p, %struct.person_t** %pt, align 8, !dbg !54
  %2 = load %struct.person_t*, %struct.person_t** %pt, align 8, !dbg !55
  call void @f(%struct.person_t* %2), !dbg !56
  ret i32 0, !dbg !57
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
!1 = !DIFile(filename: "test_complex_struct_passing.c", directory: "/home/yongzhe/llvm-versions/llvm-5.0/lib/Analysis/CDG/test/test_parameter_tree")
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
!35 = !DILocalVariable(name: "color", scope: !7, file: !1, line: 17, type: !31)
!36 = !DILocation(line: 17, column: 11, scope: !7)
!37 = !DILocation(line: 17, column: 19, scope: !7)
!38 = !DILocation(line: 17, column: 23, scope: !7)
!39 = !DILocation(line: 17, column: 26, scope: !7)
!40 = !DILocation(line: 19, column: 39, scope: !7)
!41 = !DILocation(line: 19, column: 45, scope: !7)
!42 = !DILocation(line: 19, column: 5, scope: !7)
!43 = !DILocation(line: 20, column: 1, scope: !7)
!44 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 22, type: !45, isLocal: false, isDefinition: true, scopeLine: 22, isOptimized: false, unit: !0, variables: !2)
!45 = !DISubroutineType(types: !46)
!46 = !{!15}
!47 = !DILocalVariable(name: "c", scope: !44, file: !1, line: 23, type: !23)
!48 = !DILocation(line: 23, column: 13, scope: !44)
!49 = !DILocalVariable(name: "p", scope: !44, file: !1, line: 24, type: !11)
!50 = !DILocation(line: 24, column: 12, scope: !44)
!51 = !DILocation(line: 24, column: 16, scope: !44)
!52 = !DILocation(line: 24, column: 21, scope: !44)
!53 = !DILocalVariable(name: "pt", scope: !44, file: !1, line: 25, type: !10)
!54 = !DILocation(line: 25, column: 13, scope: !44)
!55 = !DILocation(line: 26, column: 7, scope: !44)
!56 = !DILocation(line: 26, column: 5, scope: !44)
!57 = !DILocation(line: 28, column: 5, scope: !44)
