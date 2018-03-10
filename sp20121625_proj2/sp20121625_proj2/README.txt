Program is written by C with gcc
write make to compile
read document for detail

test files
												run
1. no START on asm code							 O
2. invalid operand for START directive		     X
5. no operand for START directive				 X

6. no END directive on asm code                  O
7. oprand for end is not in symbol table         X
8. no operand for END directive					 X
9. invalid operand for END directive			 X
10. address in symbol table is not start address X

11. invalid range for WORD directive			 X
12. invalid range for WORD directive(negative)	 X
13. invalid operand for WORD directive			 X
14. no operand for WORD directive				 X
15. invalid operand for WORD directive(,)	     X
16. invalid operand for WORD directive(')		 X

17. invalid operand for RESW directive			 X
18. no operand for RESW directive				 X
19. invalid operand for RES directive(,)	     X
20. invalid operand for RES directive(')		 X

21. invalid operand for RESB directive			 X
22. no operand for RESB directive				 X
23. invalid operand for RES directive(,)	     X
24. invalid operand for RES directive(')		 X

25. not X nor C for BYTE directive				 X
26. not X nor C for BYTE directive(empty)		 X
27. invalid format CEOF'						 X
28. invalid format X'0 5'						 X
29. invalid format C'EO'F						 X
30. invalid format X'05' ,						 X
31. X,'05'										 X
32. CE'OF'										 X
33. C,'EOF'										 X
34. unnecessary label for BASE directive		 O
35. no operand for BASE directive				 X
36. invalid operand for BASE (,)                 X
37. invalid operand for BASE ( ,)                X
38. unnecessary operand for format 1             X
39. invalid format for format 1                  X
40. @constant for format 3/4                     X
41. no operand for format 3/4					 X
42. invalid combination (#c ,X)					 X
43. invalid format (m ,C)						 X
44. invalid format for format 2(COMPR  A, K)	 X
45. invalid format for format 2(COMPR  A, S k)	 X
46. invalid combination (@m ,X)					 X
47. invalid format ( BUFFER,, X)				 X
48. invalid format ( STCH   BUFFER, )			 X
 
