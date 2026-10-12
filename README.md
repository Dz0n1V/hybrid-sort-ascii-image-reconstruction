# Hybrid Sort ASCII Image Reconstruction

A C++17 command-line tool that reconstructs scrambled ASCII-art images by sorting their pixels with **hybrid sorting algorithms**, and benchmarks them against `std::sort`.

Every pixel of an encoded image carries an integer index and a single ASCII character. The pixels were shuffled, and the original image is recovered by sorting them back into ascending index order. The sorting algorithms are only allowed to interact with the image through two primitives, `getElement(i)` and `swapElements(i, j)`, which makes the problem a nice playground for in-place sorting under tight constraints.

```text
Scrambled pixels  ──►  Sorting algorithm  ──►  Reconstructed ASCII image
 (index, char)         (Merge+Insertion,
                        Quick+Bubble,
                        std::sort)
```

## Features

- **Hybrid Merge Sort + Insertion Sort**: merge sort until a partition holds `k` elements or fewer, then insertion sort on the small partitions.
- **Hybrid Quick Sort + Bubble Sort**: quick sort until the number of partition splits reaches `c · log₂(n)`, then bubble sort on the remaining partition.
- **`std::sort` baseline** adapted to the restricted `Image` interface.
- **Fully iterative**: both hybrids use an explicit stack instead of recursion, so there is no risk of call-stack overflow.
- **In-place**: sorting is performed only with `swapElements`, with no auxiliary pixel buffer in the hybrid algorithms.
- **Interactive menu** with user-defined tuning constants (`k`, `c`) and an automated benchmark across all input files.

## How the algorithms work

### Hybrid Merge + Insertion (`HybridMergeInsertion`)

1. An explicit stack holds ranges together with a flag that tells whether the range is to be *split* or *merged*.
2. A range of size `≤ k` is sorted directly with insertion sort.
3. A larger range is split in half; both halves are pushed, followed by a deferred "merge" entry for the parent range.
4. Since only `swapElements` is available, the merge is done **in place** by shifting elements with adjacent swaps. An early exit skips the merge when the two halves are already in order, so sorted input is handled in near-linear time.

### Hybrid Quick + Bubble (`HybridQuickBubble`)

1. An explicit stack holds ranges together with their split depth.
2. While the depth is below `⌊c · log₂(n)⌋`, the range is partitioned with a **randomized pivot** and Hoare-style partitioning, and both sides are pushed with `depth + 1`.
3. Once the depth limit is reached, the range is finished with bubble sort (with an early-exit flag).
4. The depth limit bounds how far quick sort recurses, and bubble sort finishes the small, nearly sorted ranges.

### `std::sort` baseline (`StdLibSort`)

The indices are copied into a `std::vector`, sorted with `std::sort`, and the resulting permutation is applied back to the image using `swapElements` (tracking positions with a map).

## Project structure

```text
.
├── src/
│   ├── main.cpp             # Sorting algorithms, menu, benchmark
│   ├── image.h              # Image / Pixel (provided interface)
│   ├── image_processor.h    # Image loading and printing (provided interface)
│   ├── sorting.h            # SortingAlgorithm base class (provided interface)
│   └── utils.h              # Stopwatch (provided interface)
├── data/                    # Encoded input images (12 files)
│   ├── image_{1..4}_asc.txt
│   ├── image_{1..4}_desc.txt
│   └── image_{1..4}_rand.txt
└── README.md
```

## Input format

Each encoded image is a text file:

```text
<rows>
<cols>
<index>,<character>
<index>,<character>
...
```

There are `rows × cols` pixel lines. The four test images (all 120 columns wide) are provided in three initial orderings each: already sorted (`asc`), reverse sorted (`desc`) and randomly shuffled (`rand`).

| Image | Size (rows × cols) | Pixels |
|:-----:|:------------------:|-------:|
| 1 | 57 × 120 | 6,840 |
| 2 | 92 × 120 | 11,040 |
| 3 | 100 × 120 | 12,000 |
| 4 | 109 × 120 | 13,080 |

## Build and run

**Requirements:** a C++17 compiler (GCC, Clang or MSVC).

```bash
# Build
mkdir -p build
g++ -std=c++17 -O2 -o build/reconstruct src/main.cpp

# Run from the data directory: input files are read from, and
# output files are written to, the current working directory
cd data
../build/reconstruct
```

On Windows (MinGW), use `build\reconstruct.exe`.

### Menu

| Option | Action |
|:------:|--------|
| 1 | Reconstruct all images with Merge + Insertion (asks for `k`) |
| 2 | Reconstruct all images with Quick + Bubble (asks for `c`) |
| 3 | Benchmark both hybrids against `std::sort` (asks for `k` and `c`) |
| 4 | Exit |

Reconstructed images are written next to the inputs as `<name>_sorted.txt`, e.g. `image_1_rand_sorted.txt`.

## Benchmark

Run with `k = 16`, `c = 2`. The numbers are the median of 5 runs, in milliseconds, measured on Linux with `g++ 13 -O2`. They cover the **whole** reconstruction pipeline (loading, sorting and writing the image), so very small values are dominated by file I/O. Your numbers will differ with hardware and parameters.

| Image | Merge + Insertion | Quick + Bubble | `std::sort` |
|-------|------------------:|---------------:|------------:|
| image_1_asc  |  2 | 1 | 1 |
| image_1_desc | 24 | 1 | 1 |
| image_1_rand | 12 | 1 | 3 |
| image_2_asc  |  0 | 1 | 1 |
| image_2_desc | 59 | 1 | 2 |
| image_2_rand | 31 | 2 | 5 |
| image_3_asc  |  1 | 1 | 2 |
| image_3_desc | 71 | 1 | 2 |
| image_3_rand | 37 | 2 | 5 |
| image_4_asc  |  1 | 1 | 2 |
| image_4_desc | 85 | 1 | 2 |
| image_4_rand | 44 | 2 | 6 |

### Takeaways

- **Quick + Bubble** is consistently on par with or faster than `std::sort` here, regardless of the initial ordering. The randomized pivot avoids the classic worst case on already sorted or reverse-sorted input.
- **Merge + Insertion** is excellent on sorted input thanks to the early-exit merge check, but degrades on reversed and random input. The restriction to `swapElements` forces an in-place merge based on shifting, which is quadratic in the worst case, whereas a classic merge sort would need an auxiliary buffer.
- Tuning `k` and `c` changes the results noticeably, which is why both are runtime parameters.

## Verification

All 12 inputs were reconstructed with both hybrid algorithms and checked against the expected image: the three orderings of each picture produce byte-identical output, and the output matches the pre-sorted `asc` file. The program was also run under AddressSanitizer and UndefinedBehaviorSanitizer with a range of `k` and `c` values (including `k = 1`, `c = 0` and very large values) without any reported issues in the sorting code.

## Notes

- `image.h`, `image_processor.h`, `sorting.h` and `utils.h` define the fixed interface the algorithms are written against and are intentionally left unmodified.

## License

Distributed under the MIT License. See [`LICENSE`](LICENSE) for details.

## Technologies

C++17 · STL (`std::stack`, `std::vector`, `std::map`, `std::sort`, `<chrono>`) · iterative algorithm design · in-place sorting · benchmarking
