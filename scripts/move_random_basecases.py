#!/usr/bin/env python3
"""Move a random 10% of files from instances/pmstr-basecases to instances/.

Usage examples:
  python3 scripts/move_random_basecases.py            # move ~10%
  python3 scripts/move_random_basecases.py --dry-run  # list selection only
  python3 scripts/move_random_basecases.py --percent 5 --seed 42
"""
import argparse
import os
import random
import shutil
import math
import sys


def parse_args():
    p = argparse.ArgumentParser(description="Move a random subset of basecase files")
    p.add_argument("--src", default="basecases", help="source directory")
    p.add_argument("--dst", default="instances", help="destination directory")
    p.add_argument("--percent", type=float, default=10.0, help="percentage of files to move (default 10)")
    p.add_argument("--seed", type=int, default=None, help="random seed for reproducibility")
    p.add_argument("--dry-run", action="store_true", help="only show which files would be moved")
    return p.parse_args()


def main():
    args = parse_args()

    if not os.path.isdir(args.src):
        print(f"Fonte não existe: {args.src}", file=sys.stderr)
        sys.exit(1)

    if not os.path.isdir(args.dst):
        print(f"Destino não existe, criando: {args.dst}")
        os.makedirs(args.dst, exist_ok=True)

    files = [f for f in os.listdir(args.src) if os.path.isfile(os.path.join(args.src, f))]
    n = len(files)
    if n == 0:
        print(f"Nenhum arquivo encontrado em {args.src}")
        return

    k = max(1, math.ceil(n * args.percent / 100.0))
    rng = random.Random(args.seed)
    selected = rng.sample(files, min(k, n))

    if args.dry_run:
        print("Seleção (dry-run):")
        for f in selected:
            print(f)
        print(f"Total: {len(selected)} de {n} arquivos (percent={args.percent}%)")
        return

    for f in selected:
        srcp = os.path.join(args.src, f)
        dstp = os.path.join(args.dst, f)
        base, ext = os.path.splitext(f)
        i = 1
        while os.path.exists(dstp):
            dstp = os.path.join(args.dst, f"{base}_{i}{ext}")
            i += 1
        shutil.move(srcp, dstp)
        print(f"Movido: {srcp} -> {dstp}")


if __name__ == "__main__":
    main()
