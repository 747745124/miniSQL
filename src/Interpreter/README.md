# Interpreter

输入字符串带空格有bug

比如
```sql
insert into person values("Alex  D", 199);
```

```
Alex  D
```
就变成了
```
Alex D
```

任意多空格都会变成一个

因为一开始没考虑到有空格的字符串，后来看测试数据根本没有带空格的字符串，将就一下。

*怎么github的行内代码也会把多个空格合并成一个...*
```
`A       B` -> `A B`
```
`A       B` -> `A B`
