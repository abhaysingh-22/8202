import sys
import json
import subprocess
import os
import math
import heapq
import itertools
import random
from collections import defaultdict, deque

# helper for euclidean distance
def get_distance(p1, p2):
    return math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2)

# circle overlap calculation
def circle_overlap(A, B, t_dep, d, center, r, T_start, T_end):
    if d < 1e-9:
        dist_sq = (A[0] - center[0])**2 + (A[1] - center[1])**2
        if dist_sq <= r**2:
            t_enter = t_dep
            t_exit = t_dep
            if max(t_enter, T_start) <= min(t_exit, T_end) + 1e-9:
                return True, t_enter, t_exit
        return False, 0.0, 0.0

    vx = B[0] - A[0]
    vy = B[1] - A[1]
    dx = A[0] - center[0]
    dy = A[1] - center[1]
    
    a = vx**2 + vy**2
    b = 2 * (dx * vx + dy * vy)
    c = dx**2 + dy**2 - r**2
    
    discriminant = b**2 - 4 * a * c
    if discriminant < 0:
        return False, 0.0, 0.0
        
    sqrt_d = math.sqrt(discriminant)
    u1 = (-b - sqrt_d) / (2 * a)
    u2 = (-b + sqrt_d) / (2 * a)
    
    u_enter = max(0.0, u1)
    u_exit = min(1.0, u2)
    
    if u_enter <= u_exit + 1e-9:
        t_enter = t_dep + u_enter * d
        t_exit = t_dep + u_exit * d
        if max(t_enter, T_start) <= min(t_exit, T_end) + 1e-9:
            return True, t_enter, t_exit
            
    return False, 0.0, 0.0

# rect overlap calculation (slab method)
def rect_overlap(A, B, t_dep, d, corners, T_start, T_end):
    x_min, y_min = corners[0]
    x_max, y_max = corners[1]
    
    if d < 1e-9:
        if x_min <= A[0] <= x_max and y_min <= A[1] <= y_max:
            t_enter = t_dep
            t_exit = t_dep
            if max(t_enter, T_start) <= min(t_exit, T_end) + 1e-9:
                return True, t_enter, t_exit
        return False, 0.0, 0.0
        
    vx = B[0] - A[0]
    vy = B[1] - A[1]
    
    u_xmin, u_xmax = 0.0, 1.0
    if abs(vx) < 1e-9:
        if not (x_min <= A[0] <= x_max):
            return False, 0.0, 0.0
    else:
        u1 = (x_min - A[0]) / vx
        u2 = (x_max - A[0]) / vx
        u_xmin = min(u1, u2)
        u_xmax = max(u1, u2)
        
    u_ymin, u_ymax = 0.0, 1.0
    if abs(vy) < 1e-9:
        if not (y_min <= A[1] <= y_max):
            return False, 0.0, 0.0
    else:
        u3 = (y_min - A[1]) / vy
        u4 = (y_max - A[1]) / vy
        u_ymin = min(u3, u4)
        u_ymax = max(u3, u4)
        
    u_enter = max(0.0, u_xmin, u_ymin)
    u_exit = min(1.0, u_xmax, u_ymax)
    
    if u_enter <= u_exit + 1e-9:
        t_enter = t_dep + u_enter * d
        t_exit = t_dep + u_exit * d
        if max(t_enter, T_start) <= min(t_exit, T_end) + 1e-9:
            return True, t_enter, t_exit
            
    return False, 0.0, 0.0

# check if path segment hits any dynamic obstacle
def any_collision(A, B, t_dep, d, nfzs):
    for zone in nfzs:
        if zone['shape'] == 'circle':
            collides, t_enter, t_exit = circle_overlap(
                A, B, t_dep, d, zone['center'], zone['radius'], zone['T_start'], zone['T_end']
            )
        else:
            collides, t_enter, t_exit = rect_overlap(
                A, B, t_dep, d, zone['corners'], zone['T_start'], zone['T_end']
            )
        if collides:
            return True, t_enter, t_exit, zone
    return False, 0.0, 0.0, None

# find earliest departure using jumping algorithm
def earliest_safe(A, B, t_start, nfzs):
    t_depart = t_start
    d = get_distance(A, B)
    if d < 1e-9:
        return t_start
        
    for _ in range(50):
        collides, t_enter, t_exit, zone = any_collision(A, B, t_depart, d, nfzs)
        if not collides:
            return t_depart
            
        u_enter = (t_enter - t_depart) / d
        jump_time = zone['T_end'] - u_enter * d + 1e-5
        
        if jump_time <= t_depart:
            t_depart += 0.1
        else:
            t_depart = jump_time
            
        if t_depart > 3000:
            return None
            
    return None

def get_shortest_path(start_pos, end_pos, t_start, unique_nodes, nfzs):
    d_direct = get_distance(start_pos, end_pos)
    collides, _, _, _ = any_collision(start_pos, end_pos, t_start, d_direct, nfzs)
    if not collides:
        return [
            {'x': start_pos[0], 'y': start_pos[1], 't_arr': t_start, 't_dep': t_start},
            {'x': end_pos[0], 'y': end_pos[1], 't_arr': t_start + d_direct, 't_dep': t_start + d_direct}
        ]
    g_nodes = unique_nodes[:]
    if not any(get_distance(start_pos, n) < 1e-5 for n in g_nodes):
        g_nodes.append(start_pos)
    if not any(get_distance(end_pos, n) < 1e-5 for n in g_nodes):
        g_nodes.append(end_pos)
        
    n_count = len(g_nodes)
    start_idx = next(i for i, n in enumerate(g_nodes) if get_distance(start_pos, n) < 1e-5)
    end_idx = next(i for i, n in enumerate(g_nodes) if get_distance(end_pos, n) < 1e-5)
    
    earliest_arr = [float('inf')] * n_count
    earliest_arr[start_idx] = t_start
    
    parent = [-1] * n_count
    parent_t_dep = [0.0] * n_count
    
    pq = [(t_start, start_idx)]
    
    while pq:
        t_curr, u = heapq.heappop(pq)
        if t_curr > earliest_arr[u]:
            continue
        if u == end_idx:
            break
            
        for v in range(n_count):
            if v == u:
                continue
            t_dep = earliest_safe(g_nodes[u], g_nodes[v], t_curr, nfzs)
            if t_dep is not None:
                d = get_distance(g_nodes[u], g_nodes[v])
                arr_time = t_dep + d
                if arr_time < earliest_arr[v]:
                    earliest_arr[v] = arr_time
                    parent[v] = u
                    parent_t_dep[v] = t_dep
                    heapq.heappush(pq, (arr_time, v))
                    
    if earliest_arr[end_idx] == float('inf'):
        return None
        
    # rebuild path list
    p_nodes = []
    curr = end_idx
    while curr != -1:
        p_nodes.append((g_nodes[curr], earliest_arr[curr], parent_t_dep[curr]))
        curr = parent[curr]
    p_nodes.reverse()
    
    fmt_path = []
    for i in range(len(p_nodes)):
        pos, arr_t, dep_t = p_nodes[i]
        fmt_path.append({
            'x': pos[0],
            'y': pos[1],
            't_arr': arr_t,
            't_dep': dep_t if i < len(p_nodes) - 1 else arr_t
        })
    for i in range(len(fmt_path) - 1):
        fmt_path[i]['t_dep'] = p_nodes[i+1][2]
        
    return fmt_path

# schedule CS slots
def book_charge_slot(station_slots, cs_x, cs_y, t_arr, charge_dur):
    cs = next(c for c in station_slots if abs(c['x'] - cs_x) < 1e-5 and abs(c['y'] - cs_y) < 1e-5)
    best_t_start = float('inf')
    best_slot_idx = -1
    
    for idx, slots_data in enumerate(cs['slots']):
        t_test = t_arr
        while True:
            overlap = False
            for s, e in slots_data:
                if max(t_test, s) <= min(t_test + charge_dur, e) + 1e-9:
                    t_test = e
                    overlap = True
                    break
            if not overlap:
                break
        if t_test < best_t_start:
            best_t_start = t_test
            best_slot_idx = idx
            
    cs['slots'][best_slot_idx].append((best_t_start, best_t_start + charge_dur))
    cs['slots'][best_slot_idx].sort()
    return best_t_start

# pure Python solver as dynamic fallback
def solve_python(data):
    map_size = data['map_size']
    warehouse = (map_size[0] / 2, map_size[1] / 2)
    drones_list = data['drones']
    deliveries_list = data['deliveries']
    cs_list = data.get('charging_stations', [])
    nfzs = data.get('no_fly_zones', [])
    
    # waypoints around map
    wps = [warehouse]
    for cs in cs_list:
        wps.append((cs['x'], cs['y']))
    for dl in deliveries_list:
        wps.append((dl['x'], dl['y']))
        
    for zone in nfzs:
        if zone['shape'] == 'circle':
            xc, yc = zone['center']
            r = zone['radius'] + 2.0
            for angle in [0, 45, 90, 135, 180, 225, 270, 315]:
                rad = math.radians(angle)
                wx = xc + r * math.cos(rad)
                wy = yc + r * math.sin(rad)
                wx = max(0.1, min(map_size[0] - 0.1, wx))
                wy = max(0.1, min(map_size[1] - 0.1, wy))
                wps.append((wx, wy))
        else:
            x_min, y_min = zone['corners'][0]
            x_max, y_max = zone['corners'][1]
            x_mid = (x_min + x_max) / 2
            y_mid = (y_min + y_max) / 2
            
            pts = [
                (x_min - 2.0, y_min - 2.0),
                (x_max + 2.0, y_min - 2.0),
                (x_min - 2.0, y_max + 2.0),
                (x_max + 2.0, y_max + 2.0),
                (x_min - 2.0, y_mid),
                (x_max + 2.0, y_mid),
                (x_mid, y_min - 2.0),
                (x_mid, y_max + 2.0)
            ]
            for wx, wy in pts:
                wx = max(0.1, min(map_size[0] - 0.1, wx))
                wy = max(0.1, min(map_size[1] - 0.1, wy))
                wps.append((wx, wy))
                
    # remove duplicate locations
    unique_nodes = []
    for p in wps:
        if not any(get_distance(p, uwp) < 1e-5 for uwp in unique_nodes):
            unique_nodes.append(p)
            
    # track station slots
    station_slots = []
    for cs in cs_list:
        station_slots.append({
            'x': cs['x'],
            'y': cs['y'],
            'slots': [[] for _ in range(cs.get('slots', 1))]
        })
        
    # scheduling state
    undelivered = deliveries_list[:]
    manifest_list = []
    
    for drone in drones_list:
        drone_id = drone['id']
        max_payload = drone['max_payload']
        
        drone_manifest_path = []
        curr_time = 0.0
        curr_battery = 500.0
        
        while undelivered:
            trip_packages = []
            current_payload = 0.0
            
            # sort by deadline to deliver urgent ones
            undelivered.sort(key=lambda x: x['deadline'])
            
            for pkg in undelivered[:]:
                if current_payload + pkg['weight'] <= max_payload + 1e-9:
                    trip_packages.append(pkg)
                    current_payload += pkg['weight']
                    undelivered.remove(pkg)
                    
            if not trip_packages:
                break
                
            best_seq = None
            best_leg_paths = None
            best_charging_stations = None
            
            # permutations if small, greedy if large
            if len(trip_packages) <= 5:
                perms = list(itertools.permutations(trip_packages))
            else:
                greedy_perm = []
                current_pos = warehouse
                temp_pkgs = trip_packages[:]
                while temp_pkgs:
                    best_next = None
                    best_score = float('inf')
                    for pkg in temp_pkgs:
                        d_to = get_distance(current_pos, (pkg['x'], pkg['y']))
                        score = d_to + 0.1 * pkg['deadline']
                        if score < best_score:
                            best_score = score
                            best_next = pkg
                    greedy_perm.append(best_next)
                    current_pos = (best_next['x'], best_next['y'])
                    temp_pkgs.remove(best_next)
                perms = [tuple(greedy_perm)]
                
            for perm in perms:
                targets = [warehouse]
                for p in perm:
                    targets.append((p['x'], p['y']))
                targets.append(warehouse)
                
                leg_paths = []
                leg_costs = []
                feasible = True
                t_curr = curr_time
                bat_curr = 500.0 if not drone_manifest_path else curr_battery
                
                payloads = []
                w_remaining = sum(p['weight'] for p in perm)
                for p in perm:
                    payloads.append(w_remaining)
                    w_remaining -= p['weight']
                payloads.append(0.0)
                
                for i in range(len(targets) - 1):
                    p_leg = get_shortest_path(targets[i], targets[i+1], t_curr, unique_nodes, nfzs)
                    if p_leg is None:
                        feasible = False
                        break
                    leg_paths.append(p_leg)
                    leg_dist = sum(get_distance((p_leg[j]['x'], p_leg[j]['y']), (p_leg[j+1]['x'], p_leg[j+1]['y'])) for j in range(len(p_leg)-1))
                    leg_energy = leg_dist * (1.0 + payloads[i])
                    leg_costs.append(leg_energy)
                    t_curr = p_leg[-1]['t_arr']
                    
                if not feasible:
                    continue
                    
                t_curr = curr_time
                bat_curr = 500.0 if not drone_manifest_path else curr_battery
                leg_paths_with_charge = []
                charging_inserts = {}
                
                for i in range(len(targets) - 1):
                    if bat_curr >= leg_costs[i]:
                        bat_curr -= leg_costs[i]
                        leg_paths_with_charge.append(leg_paths[i])
                        t_curr = leg_paths[i][-1]['t_arr']
                    else:
                        best_cs = None
                        best_cs_t_completion = float('inf')
                        best_p1 = None
                        best_p2 = None
                        best_charge_dur = 0.0
                        best_wait_dur = 0.0
                        best_bat_after = 0.0
                        
                        E_remaining = sum(leg_costs[j] for j in range(i, len(targets) - 1))
                        
                        for cs in cs_list:
                            cs_pos = (cs['x'], cs['y'])
                            p1 = get_shortest_path(targets[i], cs_pos, t_curr, unique_nodes, nfzs)
                            if p1 is None:
                                continue
                            dist1 = sum(get_distance((p1[j]['x'], p1[j]['y']), (p1[j+1]['x'], p1[j+1]['y'])) for j in range(len(p1)-1))
                            cost1 = dist1 * (1.0 + payloads[i])
                            
                            if bat_curr < cost1:
                                continue
                                
                            t_arr_cs = p1[-1]['t_arr']
                            bat_arr_cs = bat_curr - cost1
                            
                            E_req_after = sum(leg_costs[j] for j in range(i+1, len(targets) - 1))
                            p2_temp = get_shortest_path(cs_pos, targets[i+1], t_arr_cs, unique_nodes, nfzs)
                            if p2_temp is None:
                                continue
                            dist2 = sum(get_distance((p2_temp[j]['x'], p2_temp[j]['y']), (p2_temp[j+1]['x'], p2_temp[j+1]['y'])) for j in range(len(p2_temp)-1))
                            cost2 = dist2 * (1.0 + payloads[i])
                            
                            bat_needed = cost2 + E_req_after
                            bat_target = min(500.0, bat_needed)
                            
                            if bat_arr_cs >= bat_target:
                                charge_dur = 0.0
                                wait_dur = 0.0
                                t_dep_cs = t_arr_cs
                            else:
                                charge_dur = (bat_target - bat_arr_cs) / 2.0
                                t_charge_start = book_charge_slot(station_slots, cs['x'], cs['y'], t_arr_cs, charge_dur)
                                wait_dur = t_charge_start - t_arr_cs
                                t_dep_cs = t_charge_start + charge_dur
                                
                            p2 = get_shortest_path(cs_pos, targets[i+1], t_dep_cs, unique_nodes, nfzs)
                            if p2 is None:
                                continue
                                
                            t_completion = p2[-1]['t_arr']
                            if t_completion < best_cs_t_completion:
                                best_cs_t_completion = t_completion
                                best_cs = cs
                                best_p1 = p1
                                best_p2 = p2
                                best_charge_dur = charge_dur
                                best_wait_dur = wait_dur
                                best_bat_after = bat_target - cost2
                                
                        if best_cs is not None:
                            charging_inserts[i] = {
                                'cs_x': best_cs['x'],
                                'cs_y': best_cs['y'],
                                'charge_dur': best_charge_dur,
                                'wait_dur': best_wait_dur,
                                'p1': best_p1,
                                'p2': best_p2
                            }
                            bat_curr = best_bat_after
                            t_curr = best_cs_t_completion
                        else:
                            feasible = False
                            break
                            
                if not feasible:
                    continue
                    
                t_check = curr_time
                for i in range(len(perm)):
                    if i in charging_inserts:
                        t_check = charging_inserts[i]['p2'][-1]['t_arr']
                    else:
                        t_check = leg_paths[i][-1]['t_arr']
                        
                    if t_check > perm[i]['deadline']:
                        feasible = False
                        break
                        
                if feasible:
                    best_seq = perm
                    best_leg_paths = leg_paths
                    best_charging_stations = charging_inserts
                    break
                    
            if best_seq is not None:
                trip_path = []
                pickup_t = curr_time
                trip_path.append({
                    'x': warehouse[0],
                    'y': warehouse[1],
                    't': pickup_t,
                    'action': 'PICKUP',
                    'delivery_ids': [p['id'] for p in best_seq]
                })
                
                t_loc = curr_time
                for i in range(len(best_seq) + 1):
                    if i in best_charging_stations:
                        insert = best_charging_stations[i]
                        p1 = insert['p1']
                        for j in range(1, len(p1)):
                            if p1[j-1]['t_dep'] > p1[j-1]['t_arr'] + 1e-9:
                                trip_path.append({
                                    'x': p1[j-1]['x'],
                                    'y': p1[j-1]['y'],
                                    't': p1[j-1]['t_arr'],
                                    'action': 'WAIT'
                                })
                                trip_path.append({
                                    'x': p1[j-1]['x'],
                                    'y': p1[j-1]['y'],
                                    't': p1[j-1]['t_dep'],
                                    'action': 'WAYPOINT'
                                })
                            if j < len(p1) - 1:
                                trip_path.append({
                                    'x': p1[j]['x'],
                                    'y': p1[j]['y'],
                                    't': p1[j]['t_arr'],
                                    'action': 'WAYPOINT'
                                })
                                
                        t_cs_arr = p1[-1]['t_arr']
                        trip_path.append({
                            'x': insert['cs_x'],
                            'y': insert['cs_y'],
                            't': t_cs_arr,
                            'action': 'CHARGE'
                        })
                        
                        if insert['wait_dur'] > 1e-9:
                            trip_path.append({
                                'x': insert['cs_x'],
                                'y': insert['cs_y'],
                                't': t_cs_arr + insert['wait_dur'],
                                'action': 'CHARGE'
                            })
                            
                        t_cs_dep = t_cs_arr + insert['wait_dur'] + insert['charge_dur']
                        trip_path.append({
                            'x': insert['cs_x'],
                            'y': insert['cs_y'],
                            't': t_cs_dep,
                            'action': 'CHARGE_COMPLETE'
                        })
                        
                        p2 = insert['p2']
                        for j in range(1, len(p2)):
                            if p2[j-1]['t_dep'] > p2[j-1]['t_arr'] + 1e-9:
                                trip_path.append({
                                    'x': p2[j-1]['x'],
                                    'y': p2[j-1]['y'],
                                    't': p2[j-1]['t_arr'],
                                    'action': 'WAIT'
                                })
                                trip_path.append({
                                    'x': p2[j-1]['x'],
                                    'y': p2[j-1]['y'],
                                    't': p2[j-1]['t_dep'],
                                    'action': 'WAYPOINT'
                                })
                            if j < len(p2) - 1:
                                trip_path.append({
                                    'x': p2[j]['x'],
                                    'y': p2[j]['y'],
                                    't': p2[j]['t_arr'],
                                    'action': 'WAYPOINT'
                                })
                        t_loc = p2[-1]['t_arr']
                    else:
                        p_leg = best_leg_paths[i]
                        for j in range(1, len(p_leg)):
                            if p_leg[j-1]['t_dep'] > p_leg[j-1]['t_arr'] + 1e-9:
                                trip_path.append({
                                    'x': p_leg[j-1]['x'],
                                    'y': p_leg[j-1]['y'],
                                    't': p_leg[j-1]['t_arr'],
                                    'action': 'WAIT'
                                })
                                trip_path.append({
                                    'x': p_leg[j-1]['x'],
                                    'y': p_leg[j-1]['y'],
                                    't': p_leg[j-1]['t_dep'],
                                    'action': 'WAYPOINT'
                                })
                            if j < len(p_leg) - 1:
                                trip_path.append({
                                    'x': p_leg[j]['x'],
                                    'y': p_leg[j]['y'],
                                    't': p_leg[j]['t_arr'],
                                    'action': 'WAYPOINT'
                                })
                        t_loc = p_leg[-1]['t_arr']
                        
                    if i < len(best_seq):
                        trip_path.append({
                            'x': best_seq[i]['x'],
                            'y': best_seq[i]['y'],
                            't': t_loc,
                            'action': 'DELIVER',
                            'delivery_id': best_seq[i]['id']
                        })
                    else:
                        trip_path.append({
                            'x': warehouse[0],
                            'y': warehouse[1],
                            't': t_loc,
                            'action': 'RETURN'
                        })
                        
                drone_manifest_path.extend(trip_path)
                curr_time = t_loc
                curr_battery = 500.0
            else:
                undelivered.extend(trip_packages)
                break
                
        if drone_manifest_path:
            manifest_list.append({
                'drone_id': drone_id,
                'path': drone_manifest_path
            })
            
    output_manifest = {
        'flight_manifest': manifest_list
    }
    
    print(json.dumps(output_manifest, indent=2))

def solve():
    if len(sys.argv) >= 2:
        with open(sys.argv[1]) as file_ptr:
            data = json.load(file_ptr)
    else:
        input_data = sys.stdin.read()
        if not input_data.strip():
            return
        data = json.loads(input_data)
        
    cpp_source = 'gsih03.cpp'
    cpp_binary = './gsih03_cpp'
    
    # Try compiling C++ file if it exists and binary does not
    if not os.path.exists(cpp_binary) and os.path.exists(cpp_source):
        try:
            subprocess.run(['g++', '-O3', '-std=c++17', cpp_source, '-o', cpp_binary], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except Exception:
            pass
            
    # Try running C++ solver
    if os.path.exists(cpp_binary):
        try:
            res = subprocess.run([cpp_binary], input=json.dumps(data), text=True, capture_output=True)
            if res.returncode == 0:
                output_json = json.loads(res.stdout)
                # print formatted JSON back to stdout
                print(json.dumps(output_json, indent=2))
                return
        except Exception:
            pass
            
    # Fallback to the Python solver
    solve_python(data)

if __name__ == '__main__':
    solve()
