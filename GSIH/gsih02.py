import sys
import math
from collections import defaultdict, deque

def solve():
    raw_input = sys.stdin.read().split()
    if not raw_input:
        return
    
    tokens = iter(raw_input)
    
    # read N, D, H
    n_ppl = int(next(tokens))
    days_cnt = int(next(tokens))
    hours_limit = int(next(tokens))
    
    # read travelers
    travelers = []
    for _ in range(n_ppl):
        name = next(tokens)
        b = int(next(tokens))
        e = int(next(tokens))
        k = int(next(tokens))
        interests = set()
        for _ in range(k):
            interests.add(next(tokens))
        travelers.append({
            'name': name,
            'budget': b,
            'energy': e,
            'tags': interests
        })
        
    # read activities
    num_acts = int(next(tokens))
    activities = []
    for _ in range(num_acts):
        aid = int(next(tokens))
        aname = next(tokens)
        acost = int(next(tokens))
        adur = int(next(tokens))
        aenergy = int(next(tokens))
        atag = next(tokens)
        activities.append({
            'id': aid,
            'name': aname,
            'cost': acost,
            'dur': adur,
            'energy': aenergy,
            'tag': atag
        })
    # sort by id so subset search naturally gets sorted lists
    activities.sort(key=lambda x: x['id'])
    
    # read events
    try:
        num_events = int(next(tokens))
    except StopIteration:
        num_events = 0
        
    events_list = []
    for _ in range(num_events):
        ev_type = next(tokens)
        ev_day = int(next(tokens))
        if ev_type == 'WEATHER':
            tag = next(tokens)
            events_list.append(('WEATHER', ev_day, tag))
        elif ev_type == 'DROP':
            pname = next(tokens)
            events_list.append(('DROP', ev_day, pname))
        elif ev_type == 'FATIGUE':
            pname = next(tokens)
            val = int(next(tokens))
            events_list.append(('FATIGUE', ev_day, pname, val))
        elif ev_type == 'BUDGET':
            pname = next(tokens)
            val = int(next(tokens))
            events_list.append(('BUDGET', ev_day, pname, val))

    # check who is active on a given day
    def get_active_users(day, active_events):
        active = []
        for p in travelers:
            name = p['name']
            budget = p['budget']
            energy = p['energy']
            tags = p['tags']
            dropped = False
            for ev in active_events:
                ev_type, ev_day = ev[0], ev[1]
                if ev_day <= day:
                    if ev_type == 'DROP' and ev[2] == name:
                        dropped = True
                    elif ev_type == 'FATIGUE' and ev[2] == name:
                        energy = ev[3]
                    elif ev_type == 'BUDGET' and ev[2] == name:
                        budget = ev[3]
            if not dropped:
                active.append((name, budget, energy, tags))
        return active

    def get_weather_blocks(day, active_events):
        blocks = set()
        for ev in active_events:
            if ev[0] == 'WEATHER' and ev[1] == day:
                blocks.add(ev[2])
        return blocks

    # bitmask subset optimizer
    def optimize_day(eligible, min_b, min_e, max_h, active_users):
        n = len(eligible)
        if n == 0:
            return [], 0, 0
            
        costs = [act['cost'] for act in eligible]
        energies = [act['energy'] for act in eligible]
        durs = [act['dur'] for act in eligible]
        ids = [act['id'] for act in eligible]
        
        # calculate satisfaction score for each activity
        sats = []
        for act in eligible:
            cnt = 0
            for _, _, _, tags in active_users:
                if act['tag'] in tags:
                    cnt += 1
            sats.append(cnt)
            
        best_sat = 0
        best_cost = 0
        best_ids = []
        
        for mask in range(1, 1 << n):
            c_sum, e_sum, d_sum, s_sum = 0, 0, 0, 0
            valid = True
            for i in range(n):
                if (mask >> i) & 1:
                    c_sum += costs[i]
                    e_sum += energies[i]
                    d_sum += durs[i]
                    s_sum += sats[i]
                    if c_sum > min_b or e_sum > min_e or d_sum > max_h:
                        valid = False
                        break
            if not valid:
                continue
                
            # lexicographical comparison
            if s_sum > best_sat:
                best_sat = s_sum
                best_cost = c_sum
                best_ids = [ids[j] for j in range(n) if (mask >> j) & 1]
            elif s_sum == best_sat and c_sum < best_cost:
                best_sat = s_sum
                best_cost = c_sum
                best_ids = [ids[j] for j in range(n) if (mask >> j) & 1]
            elif s_sum == best_sat and c_sum == best_cost:
                curr_ids = [ids[j] for j in range(n) if (mask >> j) & 1]
                if not best_ids or curr_ids < best_ids:
                    best_sat = s_sum
                    best_cost = c_sum
                    best_ids = curr_ids
                    
        return best_ids, best_cost, best_sat

    def run_planner(start_d, used_ids, active_events):
        plan = {}
        used = set(used_ids)
        for d in range(start_d, days_cnt + 1):
            ppl = get_active_users(d, active_events)
            if not ppl:
                plan[d] = ([], 0, 0)
                continue
                
            blocked = get_weather_blocks(d, active_events)
            min_b = min(p[1] for p in ppl)
            min_e = min(p[2] for p in ppl)
            
            eligible = [a for a in activities if a['id'] not in used and a['tag'] not in blocked]
            
            chosen_ids, cost, sat = optimize_day(eligible, min_b, min_e, hours_limit, ppl)
            for x in chosen_ids:
                used.add(x)
            plan[d] = (chosen_ids, cost, sat)
        return plan

    def fmt(day, chosen_ids, cost, sat):
        if chosen_ids:
            return f"Day {day}: {' '.join(str(x) for x in chosen_ids)} | cost={cost} satisfaction={sat}"
        return f"Day {day}: REST | cost=0 satisfaction=0"

    # print the initial plan
    curr_plan = run_planner(1, set(), [])
    print("=== PLAN ===")
    for d in range(1, days_cnt + 1):
        print(fmt(d, *curr_plan[d]))
        
    # handle events and replan
    for idx in range(len(events_list)):
        ev = events_list[idx]
        ev_type, ev_day = ev[0], ev[1]
        
        if ev_type == 'WEATHER':
            ev_str = f"WEATHER {ev_day} {ev[2]}"
        elif ev_type == 'DROP':
            ev_str = f"DROP {ev_day} {ev[2]}"
        elif ev_type == 'FATIGUE':
            ev_str = f"FATIGUE {ev_day} {ev[2]} {ev[3]}"
        elif ev_type == 'BUDGET':
            ev_str = f"BUDGET {ev_day} {ev[2]} {ev[3]}"
            
        print(f"=== EVENT {idx+1}: {ev_str} ===")
        
        already_used = set()
        for d in range(1, min(ev_day, days_cnt + 1)):
            for aid in curr_plan[d][0]:
                already_used.add(aid)
                
        if ev_day <= days_cnt:
            new_plan = run_planner(ev_day, already_used, events_list[:idx+1])
            for d in range(ev_day, days_cnt + 1):
                curr_plan[d] = new_plan[d]
                
        for d in range(ev_day, days_cnt + 1):
            print(fmt(d, *curr_plan[d]))

if __name__ == '__main__':
    solve()
