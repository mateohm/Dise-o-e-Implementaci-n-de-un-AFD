import sys
import re
from typing import Dict, Set, Tuple

COMMENT_RE = re.compile(r"^\s*#")

def parse_conf(path: str):
    states: Set[str] = set()
    alphabet: Set[str] = set()
    start: str | None = None
    accept: Set[str] = set()
    delta: Dict[Tuple[str, str], str] = {}

    with open(path, 'r', encoding='utf-8') as f:
        lines = [ln.strip() for ln in f]

    # filtra comentarios y vacíos
    lines = [ln for ln in lines if ln and not COMMENT_RE.match(ln)]

    i = 0
    while i < len(lines):
        ln = lines[i]
        if ln.lower().startswith('states:'):
            vals = ln.split(':', 1)[1]
            states = {s.strip() for s in vals.split(',') if s.strip()}
        elif ln.lower().startswith('alphabet:'):
            vals = ln.split(':', 1)[1]
            alpha_list = [s.strip() for s in vals.split(',') if s.strip()]
            for s in alpha_list:
                if len(s) != 1:
                    raise ValueError(f"Símbolo '{s}' debe ser de un carácter")
            alphabet = set(alpha_list)
        elif ln.lower().startswith('start:'):
            start = ln.split(':', 1)[1].strip()
        elif ln.lower().startswith('accept:'):
            vals = ln.split(':', 1)[1]
            accept = {s.strip() for s in vals.split(',') if s.strip()}
        elif ln.lower().startswith('transitions:'):
            i += 1
            while i < len(lines):
                tln = lines[i].strip()
                if not tln or COMMENT_RE.match(tln):
                    i += 1
                    continue
                # formato: q, a -> p
                parts = tln.split('->')
                if len(parts) != 2:
                    raise ValueError(f"Transición inválida: {tln}")
                left, right = parts[0].strip(), parts[1].strip()
                if ',' not in left:
                    raise ValueError(f"Transición inválida (falta coma): {tln}")
                st, sym = [x.strip() for x in left.split(',', 1)]
                if len(sym) != 1:
                    raise ValueError(f"Símbolo '{sym}' debe ser de un carácter")
                delta[(st, sym)] = right
                i += 1
            break  # fin del archivo
        i += 1

    if not states:
        raise ValueError("'states' vacío o ausente")
    if not alphabet:
        raise ValueError("'alphabet' vacío o ausente")
    if start is None or start not in states:
        raise ValueError("'start' inválido o ausente")
    if not accept.issubset(states):
        raise ValueError("Algún estado en 'accept' no pertenece a 'states'")

    return states, alphabet, start, accept, delta

def simulate(word: str, start: str, accept: Set[str], delta: Dict[Tuple[str, str], str], alphabet: Set[str]):
    q = start
    for ch in word:
        if ch not in alphabet:
            return False
        nxt = delta.get((q, ch))
        if nxt is None:
            return False
        q = nxt
    return q in accept

def main():
    if len(sys.argv) not in (1, 3):
        print("Uso: python dfa.py [Conf.txt Cadenas.txt]")
        sys.exit(1)
    conf_path = sys.argv[1] if len(sys.argv) == 3 else 'Conf.txt'
    words_path = sys.argv[2] if len(sys.argv) == 3 else 'Cadenas.txt'

    states, alphabet, start, accept, delta = parse_conf(conf_path)

    with open(words_path, 'r', encoding='utf-8') as f:
        for raw in f:
            s = raw.strip('\n')
            if not s.strip() or COMMENT_RE.match(s):
                continue
            ok = simulate(s.strip(), start, accept, delta, alphabet)
            print(f"{s}: {'ACEPTADA' if ok else 'RECHAZADA'}")

if __name__ == '__main__':
    main()
