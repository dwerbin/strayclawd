"""Turn the raw frames written by render_poses.exe --clips into an animated GIF.

Usage: python make_gif.py frames.bin out.gif [delay_ms]
"""
import struct
import sys
from collections import Counter


def read_frames(path):
    with open(path, 'rb') as f:
        w, h, n = struct.unpack('<iii', f.read(12))
        size = w * h * 3
        frames = [f.read(size) for _ in range(n)]
    return w, h, frames


def build_palette(frames, w, h):
    counts = Counter()
    for fr in frames:
        for i in range(0, len(fr), 3):
            counts[fr[i:i + 3]] += 1
    colors = [c for c, _ in counts.most_common(256)]
    index = {c: i for i, c in enumerate(colors)}
    cache = {}

    def lookup(c):
        if c in index:
            return index[c]
        if c not in cache:
            r, g, b = c
            cache[c] = min(range(len(colors)),
                           key=lambda i: (colors[i][0] - r) ** 2 + (colors[i][1] - g) ** 2 + (colors[i][2] - b) ** 2)
        return cache[c]

    indexed = []
    for fr in frames:
        indexed.append(bytes(lookup(fr[i:i + 3]) for i in range(0, len(fr), 3)))
    return colors, indexed


def lzw(data, min_size=8):
    clear = 1 << min_size
    end = clear + 1
    out = bytearray()
    state = {'bits': 0, 'nbits': 0}

    def emit(code, size):
        state['bits'] |= code << state['nbits']
        state['nbits'] += size
        while state['nbits'] >= 8:
            out.append(state['bits'] & 0xFF)
            state['bits'] >>= 8
            state['nbits'] -= 8

    def reset():
        return {bytes([i]): i for i in range(clear)}, end + 1, min_size + 1

    table, nxt, size = reset()
    emit(clear, size)
    w = bytes([data[0]])
    for k in data[1:]:
        wk = w + bytes([k])
        if wk in table:
            w = wk
            continue
        emit(table[w], size)
        if nxt < 4096:
            table[wk] = nxt
            nxt += 1
            if nxt > (1 << size) and size < 12:
                size += 1
        else:
            emit(clear, size)
            table, nxt, size = reset()
        w = bytes([k])
    emit(table[w], size)
    emit(end, size)
    if state['nbits']:
        out.append(state['bits'] & 0xFF)
    return bytes(out)


def sub_blocks(data):
    out = bytearray()
    for i in range(0, len(data), 255):
        chunk = data[i:i + 255]
        out.append(len(chunk))
        out += chunk
    out.append(0)
    return bytes(out)


def changed_box(prev, cur, w, h):
    if prev is None:
        return 0, 0, w, h
    x0, y0, x1, y1 = w, h, -1, -1
    for y in range(h):
        row = y * w
        a = prev[row:row + w]
        b = cur[row:row + w]
        if a == b:
            continue
        xs = [x for x in range(w) if a[x] != b[x]]
        x0, x1 = min(x0, xs[0]), max(x1, xs[-1])
        y0, y1 = min(y0, y), max(y1, y)
    if x1 < 0:
        return 0, 0, 1, 1
    return x0, y0, x1 - x0 + 1, y1 - y0 + 1


def write_gif(path, w, h, colors, frames, delay_ms):
    palette = bytearray()
    for c in colors:
        palette += bytes(c)
    palette += bytes(3 * (256 - len(colors)))
    out = bytearray(b'GIF89a')
    out += struct.pack('<HHBBB', w, h, 0xF7, 0, 0)
    out += palette
    out += b'\x21\xFF\x0BNETSCAPE2.0\x03\x01\x00\x00\x00'
    prev = None
    for fr in frames:
        x, y, bw, bh = changed_box(prev, fr, w, h)
        pixels = b''.join(fr[(y + j) * w + x:(y + j) * w + x + bw] for j in range(bh))
        out += b'\x21\xF9\x04' + struct.pack('<BHBB', 0x04, delay_ms // 10, 0, 0)
        out += b'\x2C' + struct.pack('<HHHHB', x, y, bw, bh, 0)
        out += b'\x08' + sub_blocks(lzw(pixels))
        prev = fr
    out += b'\x3B'
    with open(path, 'wb') as f:
        f.write(out)
    return len(out)


def main():
    src, dst = sys.argv[1], sys.argv[2]
    delay = int(sys.argv[3]) if len(sys.argv) > 3 else 100
    w, h, frames = read_frames(src)
    colors, indexed = build_palette(frames, w, h)
    size = write_gif(dst, w, h, colors, indexed, delay)
    print(f'{dst}: {len(frames)} frames, {w}x{h}, {len(colors)} colors, {size} bytes')


if __name__ == '__main__':
    main()
