У нас есть DOMFile. Он получает путь до dom.dbx и хранит XML ноды по страницам - page_header | node-data1 ... node-dataN |
Создает файл если его нет.

## Документное дерево (Document-oriented database)
похоже на key-value
- в нем нет отношений. Только родитель-ребенок
- в нем нет схемы??? Что такое схема?
- получение данных через query запрос


## Storage
- В файле страницы представляются как последовательный массив данных
- В оперативной памяти страницы разбросаны по памяти. 
  - Cache: загруженные страницы можно не брать с диска
  - No cache: мы не храним страницы в памяти, а только обращаемся к диску для каждой страницы
    - делаем mmap на существующую страницу, вместо malloc-а
    - делаем munmap на существующую страницу, вместо free-а???
    - при создании страницы делаем malloc
  

## Memory management
- *dirty page* - если данные изменились в странице после загрузки с диска
  - *cleaning* - запись измененных страниц на диск
- [Free page queue](https://en.wikipedia.org/wiki/Page_replacement_algorithm) - очередь с пустыми страницами. 
  - Похожий подход - *free lists* в [slab allocator](https://en.wikipedia.org/wiki/Slab_allocation)
- *smart pointer*
- (Memory pool implementation)[https://en.wikipedia.org/wiki/Memory_pool#Simple_memory_pool_implementation]
- Как связать данные из файла и страницы в оперативной памяти?
