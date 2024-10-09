## Changelog

## v1.5.0 - 2023.01.27
 + provide LinkedList deep-copy via assign operator
 + some tests cleanup
 + test cases cover `exist()` method
 * optimized `clear()` method to use non-copy `unlink()`
 * refactoring for `remove()`, `shift()`, `pop()` methods
 + added `unlink()` method to delete nodes without data object copy/return
 * fixed bug when cached node index was updated wrong on `add()`
 * change type of `size` and `index` to unsigned
 + pass data objects by const ref to avoid useless obj copy

 