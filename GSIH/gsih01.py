import sys, json

def solve(root_name, raw):
    data = json.loads(raw)

    def new_node():
        return {"keys": {}, "n": 0}

    def ensure_key(node, k):
        if k not in node["keys"]:
            node["keys"][k] = {
                "prim": set(),   
                "arr_elem": set(), 
                "cnt": 0,
                "child": None,
                "has_arr": False,
                "arr_has_obj": False
            }

    def feed(node, objs):
        node["n"] += len(objs)
        for o in objs:
            for k, v in o.items():
                ensure_key(node, k)
                rec = node["keys"][k]
                rec["cnt"] += 1
                handle_val(rec, v)

    def handle_val(rec, v):
        if v is None:
            rec["prim"].add("null")
        elif type(v) is bool:  
            rec["prim"].add("boolean")
        elif type(v) in (int, float):
            rec["prim"].add("number")
        elif type(v) is str:
            rec["prim"].add("string")
        elif type(v) is dict:
            rec["prim"].add("__obj")
            if rec["child"] is None:
                rec["child"] = new_node()
            feed(rec["child"], [v])
        elif type(v) is list:
            rec["has_arr"] = True
            for el in v:
                if type(el) is dict:
                    rec["arr_has_obj"] = True
                    if rec["child"] is None:
                        rec["child"] = new_node()
                    feed(rec["child"], [el])
                    rec["arr_elem"].add("__obj")
                elif el is None:
                    rec["arr_elem"].add("null")
                elif type(el) is bool:
                    rec["arr_elem"].add("boolean")
                elif type(el) in (int, float):
                    rec["arr_elem"].add("number")
                elif type(el) is str:
                    rec["arr_elem"].add("string")

    root = new_node()
    feed(root, data)


    used = {root_name}
    order = []

    def pick_name(base):
        if base not in used:
            used.add(base)
            return base
        i = 2
        while base + str(i) in used:
            i += 1
        used.add(base + str(i))
        return base + str(i)

    def name_pass(nd, nm):
        nd["_name"] = nm
        order.append((nm, nd))
        for k in sorted(nd["keys"]):
            info = nd["keys"][k]
            if info["child"] is not None:
                cname = pick_name(k[0].upper() + k[1:])
                name_pass(info["child"], cname)

    name_pass(root, root_name)


    def type_of(info):
        parts = set()


        if info["has_arr"]:
            elems = set()
            for t in info["arr_elem"]:
                if t == "__obj":
                    elems.add(info["child"]["_name"])
                else:
                    elems.add(t)
            if not elems:
                parts.add("unknown[]")
            elif len(elems) == 1:
                parts.add(elems.pop() + "[]")
            else:
                inner = " | ".join(sorted(elems))
                parts.add("(" + inner + ")[]")


        for t in info["prim"]:
            if t == "__obj":
                parts.add(info["child"]["_name"])
            else:
                parts.add(t)

        return " | ".join(sorted(parts)) if parts else "unknown"


    blocks = []
    for nm, nd in sorted(order):
        if not nd["keys"]:
            blocks.append(f"export interface {nm} {{}}")
            continue
        lines = [f"export interface {nm} {{"]
        for k in sorted(nd["keys"]):
            info = nd["keys"][k]
            opt = "?" if info["cnt"] < nd["n"] else ""
            lines.append(f"  {k}{opt}: {type_of(info)};")
        lines.append("}")
        blocks.append("\n".join(lines))

    return "\n\n".join(blocks) + "\n"


def main():
    inp = sys.stdin.read().split("\n")
    t = int(inp[0])
    pos = 1
    out = []
    for _ in range(t):
        rname = inp[pos].strip()
        jstr = inp[pos + 1].strip()
        pos += 2
        out.append(solve(rname, jstr))
    sys.stdout.write("---\n".join(out))

if __name__ == "__main__":
    main()
