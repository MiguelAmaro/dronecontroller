
//~ C STRING
fn b32 StrIsNullTerminated(str8 Str)
{
  b32 Result = Str.Data[Str.Length] == '\0';
  Assert(Result);
  return Result;
}
fn u64 CStrGetLength(const char *String, b32 IncludeNull)
{
  u64 Result = 0;
  while(String[Result] && Result<U32Max) { Result++; }
  Result += IncludeNull?1:0;
  return Result;
}
fn str8 Str8CopyToArena(arena *Arena, u8 *CStr)
{
  str8 Result = {0};
  u64 LengthWithNull = CStrGetLength((char *)CStr, 1);
  Result.Data = ArenaPushArray(Arena, LengthWithNull, u8);
  Result.Length = LengthWithNull - 1; //Pretend Null doesn't exist but it does
  MemoryCopy(CStr, LengthWithNull, Result.Data, LengthWithNull);
  return Result;
}
fn str8 Str8Copy(arena *Arena, str8 String)
{
  str8 Result = {0};
  Result.Data = ArenaPushArray(Arena, String.Length + 1, u8);
  Result.Length = String.Length;
  MemoryCopy(String.Data, Result.Length, Result.Data, Result.Length);
  Result.Data[Result.Length] = '\0';
  return Result;
}
fn static b32 CStrIsEqual(const char *a, const char *b)
{
  b32 Result = 1;
  size_t Length = CStrGetLength(a, 1);
  Assert(Length<3000);
  while((*a++==*b++) && (0<Length))
  {Length--;}
  Result = Length==0;
  
  return Result;
}

//~ CONNICAL STRINGS
fn str8 Str8Base(u8 *String, u64 Length)
{
  str8 Result = {0};
  Assert(Length<=U32Max);
  Result.Data = String;
  Result.Length = (u32)Length;
  return Result;
}
fn str8 Str8FromCStr(char *String)
{
  str8 Result = {0};
  Result.Data = (u8 *)String;
  Result.Length = CStrGetLength(String, 0);
  return Result;
}
fn str8 Str8FromArena(arena *Arena, u64 Length)
{
  str8 Result = {0};
  Result.Length = Length;
  Result.Data = ArenaPushArray(Arena, Length, u8);
  return Result;
}
fn b32 Str8IsEqual(str8 a, str8 b)
{
  b32 Result = 1;
  if(a.Length != b.Length) return 0;
  u64 Index = a.Length;
  while((Index>0) && (a.Data[Index-1]==b.Data[Index-1])) { Index--; }
  Result = (Index==0);
  return Result;
}
fn str8 Str8Concat(str8 a, str8 b, arena *Arena)
{
  str8 Result;
  u64 Length = a.Length + b.Length;
  u8 *Data = ArenaPushArray(Arena, Length, u8);
  MemoryCopy(a.Data,a.Length,Data,a.Length);
  MemoryCopy(b.Data,b.Length,Data+a.Length,b.Length);
  Result = Str8(Data, Length);
  return Result;
}
//
//str8 Str8InsertAt(char Char, const char *StringA, const char *StringB, arena Arena)
//{
//str8 Result = {0};
//Result.Data = (u8 *)String;
//Result.Length = CStrGetLength(String, 0);
//return Result;
//}
//

//~ STRING 32

fn str32 Str32(u32 *Data, u32 Length)
{
  str32 Result = {0};
  Result.Data = Data;
  Result.Length = Length;
  return Result;
}
//
//str32 Str32FromArena(arena Arena)
//{
//str32 Result = {0};
//Result.Data = Arena.Base;
//Result.Length = (u32)(Arena.Length/sizeof(u32));
//return Result;
//}
//

fn void BuildPrefixSuffixTable(u32* Table, str8 Pattern)
{
  u32* OriginalTable = Table;
  u32 i = 1;
  u32 j = 0;
  MemoryZero(Table, Pattern.Length*sizeof(u32));
  while(i < (Pattern.Length))
  {
    // LAST ELEMENT
    if(i == (Pattern.Length - 1))
    {
      if(Pattern.Data[i] == Pattern.Data[j] || j == 0)    { break; }
      while(Pattern.Data[i] != Pattern.Data[j] && j != 0) { j = Table[j - 1]; }
      Table[i] = ++j; i++;
    }
    // MISMATCH
    else if(Pattern.Data[i] != Pattern.Data[j])  { Table[i] = j = 0; i++; }
    else { Table[i] = j; i++; }
  }
  Assert(Table == OriginalTable);
  return;
}
fn str_search Str8Search(str8 Pattern, str8 Corpus, arena *Arena)
{
  str_search Result = {0};
  Result.Count = 0;
  Result.Found = ArenaPushArray(Arena, 64, u32);
  u32 i = 0;
  u32 j = 0;
  u32  Table[512]; 
  u32 *TableCopy = Table;
  Assert(Pattern.Length < ArrayLength(Table));
  Assert(Corpus.Data && Table);
  BuildPrefixSuffixTable(Table, Pattern);
  while(i < Corpus.Length)
  {
    // CASE: Match
    if(Pattern.Data[j] == Corpus.Data[i]) { i++; j++; }
    if(j == Pattern.Length) { Result.Found[Result.Count++] = (i - j); j = 0; }
    // CASE: Misatch
    else if((i < Corpus.Length) && (Pattern.Data[j] != Corpus.Data[i]))
    {
      if(j != 0) { j = Table[j - 1]; }
      else       { i++;              }
    }
  }
  Assert(Table == TableCopy);
  return Result;
}

