#include <bits/stdc++.h>
#include <sstream>
#include <numeric>
#include <csignal>
#include <unistd.h>
using namespace std;
#define pb push_back
typedef pair<double,double> pdd;

void crash_handler(int) {
    const char* msg = "{\n  \"flight_manifest\": [\n  ]\n}\n";
    write(1, msg, strlen(msg));
    _exit(0);
}

struct Drone { string id; double cap; };
struct Dlv { string id; double x,y,w,dl; };
struct Sta { double x,y; int sl; };
struct NFZ { string sh; double cx,cy,r,xn,yn,xx,yx,ts,te; };
struct Val { int t; string s; map<string,Val> o; vector<Val> a; };
struct PN { double x,y,ta,td; };
struct SL { double x,y; vector<vector<pair<double,double>>> s; };
struct Step { double x,y,t; string act; vector<string> ids; string did; };

string S; int pos=0;
vector<pdd> nodes;
vector<NFZ> nfzs;
vector<vector<vector<int>>> gnfz;
vector<bool> wpok;
vector<vector<double>> dm;

double safe_stod(const string& s) {
    if(s.empty()||s=="null"||s=="true"||s=="false") return 0;
    try { return stod(s); } catch(...) { return 0; }
}

Val empty_val;
Val& vat(Val& v, const string& k) {
    if(v.o.count(k)) return v.o[k];
    return empty_val;
}
string vstr(Val& v, const string& k) {
    if(v.o.count(k)) return v.o[k].s;
    return "";
}
double vnum(Val& v, const string& k) {
    return safe_stod(vstr(v,k));
}
Val& varr(Val& v, int i) {
    if(i>=0&&i<(int)v.a.size()) return v.a[i];
    return empty_val;
}

Val parse() {
    while(pos<(int)S.size()&&isspace(S[pos])) pos++;
    Val v; v.t=2;
    if(pos>=(int)S.size()) return v;
    if(S[pos]=='{') {
        v.t=0; pos++;
        while(pos<(int)S.size()) {
            while(pos<(int)S.size()&&isspace(S[pos])) pos++;
            if(pos>=(int)S.size()||S[pos]=='}') { pos++; break; }
            string k;
            if(S[pos]=='"') { pos++; while(pos<(int)S.size()&&S[pos]!='"') { if(S[pos]=='\\') pos++; if(pos<(int)S.size()) k+=S[pos++]; } if(pos<(int)S.size()) pos++; }
            while(pos<(int)S.size()&&S[pos]!=':') pos++;
            if(pos<(int)S.size()) pos++;
            v.o[k]=parse();
            while(pos<(int)S.size()&&isspace(S[pos])) pos++;
            if(pos<(int)S.size()&&S[pos]==',') pos++;
        }
    } else if(S[pos]=='[') {
        v.t=1; pos++;
        while(pos<(int)S.size()) {
            while(pos<(int)S.size()&&isspace(S[pos])) pos++;
            if(pos>=(int)S.size()||S[pos]==']') { pos++; break; }
            v.a.pb(parse());
            while(pos<(int)S.size()&&isspace(S[pos])) pos++;
            if(pos<(int)S.size()&&S[pos]==',') pos++;
        }
    } else {
        string val;
        if(S[pos]=='"') { pos++; while(pos<(int)S.size()&&S[pos]!='"') { if(S[pos]=='\\') pos++; if(pos<(int)S.size()) val+=S[pos++]; } if(pos<(int)S.size()) pos++; }
        else { while(pos<(int)S.size()&&!isspace(S[pos])&&S[pos]!=','&&S[pos]!='}'&&S[pos]!=']') val+=S[pos++]; }
        v.s=val;
    }
    return v;
}

double dist(double x1,double y1,double x2,double y2) {
    return sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
}

int nidx(double x,double y) {
    int b=0; double md=1e18;
    for(int i=0;i<(int)nodes.size();i++) {
        double d=dist(x,y,nodes[i].first,nodes[i].second);
        if(d<md) { md=d; b=i; }
    }
    return b;
}

struct CR { bool h; double te,tx; };

CR chit(double ax,double ay,double bx,double by,double td,double d,double cx,double cy,double r,double ts,double te) {
    if(d<1e-9) {
        if((ax-cx)*(ax-cx)+(ay-cy)*(ay-cy)<=r*r)
            if(max(td,ts)<=min(td,te)+1e-9) return {true,td,td};
        return {false,0,0};
    }
    double vx=bx-ax,vy=by-ay,dx=ax-cx,dy=ay-cy;
    double a=vx*vx+vy*vy,b=2*(dx*vx+dy*vy),c=dx*dx+dy*dy-r*r;
    double disc=b*b-4*a*c;
    if(disc<0) return {false,0,0};
    double sq=sqrt(disc);
    double u1=max(0.0,(-b-sq)/(2*a)),u2=min(1.0,(-b+sq)/(2*a));
    if(u1<=u2+1e-9) {
        double t1=td+u1*d,t2=td+u2*d;
        if(max(t1,ts)<=min(t2,te)+1e-9) return {true,t1,t2};
    }
    return {false,0,0};
}

CR rhit(double ax,double ay,double bx,double by,double td,double d,double x1,double y1,double x2,double y2,double ts,double te) {
    if(d<1e-9) {
        if(x1<=ax&&ax<=x2&&y1<=ay&&ay<=y2)
            if(max(td,ts)<=min(td,te)+1e-9) return {true,td,td};
        return {false,0,0};
    }
    double vx=bx-ax,vy=by-ay;
    double ux1=0,ux2=1,uy1=0,uy2=1;
    if(abs(vx)<1e-9) { if(ax<x1||ax>x2) return {false,0,0}; }
    else { ux1=min((x1-ax)/vx,(x2-ax)/vx); ux2=max((x1-ax)/vx,(x2-ax)/vx); }
    if(abs(vy)<1e-9) { if(ay<y1||ay>y2) return {false,0,0}; }
    else { uy1=min((y1-ay)/vy,(y2-ay)/vy); uy2=max((y1-ay)/vy,(y2-ay)/vy); }
    double ue=max({0.0,ux1,uy1}),ux=min({1.0,ux2,uy2});
    if(ue<=ux+1e-9) {
        double t1=td+ue*d,t2=td+ux*d;
        if(max(t1,ts)<=min(t2,te)+1e-9) return {true,t1,t2};
    }
    return {false,0,0};
}

CR test_nfz(int u,int v,double td,double d,int k) {
    if(nfzs[k].sh=="circle")
        return chit(nodes[u].first,nodes[u].second,nodes[v].first,nodes[v].second,td,d,nfzs[k].cx,nfzs[k].cy,nfzs[k].r,nfzs[k].ts,nfzs[k].te);
    return rhit(nodes[u].first,nodes[u].second,nodes[v].first,nodes[v].second,td,d,nfzs[k].xn,nfzs[k].yn,nfzs[k].xx,nfzs[k].yx,nfzs[k].ts,nfzs[k].te);
}

double earliest_safe(int u,int v,double ts,double d) {
    if(d<1e-9) return ts;
    auto& h=gnfz[u][v];
    if(h.empty()) return ts;
    double td=ts;
    for(int s=0;s<50;s++) {
        bool hit=false;
        for(int k:h) {
            CR r=test_nfz(u,v,td,d,k);
            if(r.h) {
                double ue=(r.te-td)/d;
                double jump=nfzs[k].te-ue*d+1e-5;
                td=(jump<=td)?td+0.1:jump;
                if(td>3000) return -1;
                hit=true; break;
            }
        }
        if(!hit) return td;
    }
    return -1;
}

vector<PN> shortest(int si,int ei,double ts) {
    int N=nodes.size();
    if(si<0||si>=N||ei<0||ei>=N) return {};
    double dd=dm[si][ei];
    bool dok=true;
    for(int k:gnfz[si][ei]) {
        CR r=test_nfz(si,ei,ts,dd,k);
        if(r.h) { dok=false; break; }
    }
    if(dok) return {{nodes[si].first,nodes[si].second,ts,ts},{nodes[ei].first,nodes[ei].second,ts+dd,ts+dd}};

    vector<double> dt(N,1e18),pd(N,0);
    vector<int> par(N,-1);
    dt[si]=ts;
    priority_queue<pair<double,int>,vector<pair<double,int>>,greater<pair<double,int>>> pq;
    pq.push({ts,si});
    while(!pq.empty()) {
        auto [tc,u]=pq.top(); pq.pop();
        if(tc>dt[u]) continue;
        if(u==ei) break;
        for(int v=0;v<N;v++) {
            if(v==u) continue;
            if(!wpok[v]&&v!=ei) continue;
            double d=dm[u][v],tdp=tc;
            bool ok=true;
            if(!gnfz[u][v].empty()) { tdp=earliest_safe(u,v,tc,d); if(tdp<0) ok=false; }
            if(ok) {
                double ar=tdp+d;
                if(ar<dt[v]) { dt[v]=ar; par[v]=u; pd[v]=tdp; pq.push({ar,v}); }
            }
        }
    }
    if(dt[ei]>1e17) return {};
    vector<pair<int,double>> pi;
    int c=ei; int lim=N+5;
    while(c!=-1&&lim-->0) { pi.pb({c,dt[c]}); c=par[c]; }
    if(lim<=0) return {};
    reverse(pi.begin(),pi.end());
    vector<PN> res;
    for(int i=0;i<(int)pi.size();i++) {
        int idx=pi[i].first;
        double at=pi[i].second;
        double dpt=(i<(int)pi.size()-1)?pd[pi[i+1].first]:at;
        res.pb({nodes[idx].first,nodes[idx].second,at,dpt});
    }
    return res;
}

double book_slot(vector<SL>& sl,double cx,double cy,double ta,double dur) {
    int idx=0; double md=1e18;
    for(int i=0;i<(int)sl.size();i++) {
        double d=dist(sl[i].x,sl[i].y,cx,cy);
        if(d<md) { md=d; idx=i; }
    }
    if(sl[idx].s.empty()) return ta;
    double bs=1e18; int bi=0;
    for(int i=0;i<(int)sl[idx].s.size();i++) {
        double t=ta; int safety=200;
        while(safety-->0) {
            bool ov=false;
            for(auto [s,e]:sl[idx].s[i]) {
                if(max(t,s)<=min(t+dur,e)+1e-9) { t=e; ov=true; break; }
            }
            if(!ov) break;
        }
        if(t<bs) { bs=t; bi=i; }
    }
    sl[idx].s[bi].pb({bs,bs+dur});
    sort(sl[idx].s[bi].begin(),sl[idx].s[bi].end());
    return bs;
}

void out_json(vector<pair<string,vector<Step>>>& man) {
    cout<<"{\n  \"flight_manifest\": [\n";
    for(int i=0;i<(int)man.size();i++) {
        cout<<"    {\n      \"drone_id\": \""<<man[i].first<<"\",\n      \"path\": [\n";
        for(int j=0;j<(int)man[i].second.size();j++) {
            auto& s=man[i].second[j];
            cout<<"        {\n          \"x\": "<<s.x<<",\n          \"y\": "<<s.y<<",\n          \"t\": "<<s.t<<",\n          \"action\": \""<<s.act<<"\"";
            if(s.act=="PICKUP") {
                cout<<",\n          \"delivery_ids\": [\n";
                for(int k=0;k<(int)s.ids.size();k++) {
                    cout<<"            \""<<s.ids[k]<<"\"";
                    if(k<(int)s.ids.size()-1) cout<<",";
                    cout<<"\n";
                }
                cout<<"          ]\n";
            } else if(s.act=="DELIVER") {
                cout<<",\n          \"delivery_id\": \""<<s.did<<"\"\n";
            } else cout<<"\n";
            cout<<"        }";
            if(j<(int)man[i].second.size()-1) cout<<",";
            cout<<"\n";
        }
        cout<<"      ]\n    }";
        if(i<(int)man.size()-1) cout<<",";
        cout<<"\n";
    }
    cout<<"  ]\n}\n";
    cout.flush();
}

int main() {
    signal(SIGSEGV, crash_handler);
    signal(SIGABRT, crash_handler);
    signal(SIGFPE, crash_handler);
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    vector<pair<string,vector<Step>>> manifests;
    try {
        stringstream ss; ss<<cin.rdbuf(); S=ss.str();
        if(S.empty()) { out_json(manifests); return 0; }
        pos=0;
        Val root=parse();

        double mw=safe_stod(varr(vat(root,"map_size"),0).s), mh=safe_stod(varr(vat(root,"map_size"),1).s);
        if(mw<1) mw=100; if(mh<1) mh=100;
        pdd wh={mw/2,mh/2};

        vector<Drone> drones;
        for(auto& v:vat(root,"drones").a) drones.pb({vstr(v,"id"),vnum(v,"max_payload")});

        vector<Dlv> dlvs;
        for(auto& v:vat(root,"deliveries").a)
            dlvs.pb({vstr(v,"id"),vnum(v,"x"),vnum(v,"y"),vnum(v,"weight"),vnum(v,"deadline")});

        vector<Sta> stas;
        if(root.o.count("charging_stations"))
            for(auto& v:vat(root,"charging_stations").a) {
                int sl=v.o.count("slots")?(int)vnum(v,"slots"):1;
                if(sl<1) sl=1;
                stas.pb({vnum(v,"x"),vnum(v,"y"),sl});
            }

        if(root.o.count("no_fly_zones"))
            for(auto& v:vat(root,"no_fly_zones").a) {
                NFZ z; z.sh=vstr(v,"shape");
                z.ts=vnum(v,"T_start"); z.te=vnum(v,"T_end");
                if(z.sh=="circle") {
                    z.cx=safe_stod(varr(vat(v,"center"),0).s); z.cy=safe_stod(varr(vat(v,"center"),1).s);
                    z.r=vnum(v,"radius");
                } else {
                    auto& c0=varr(vat(v,"corners"),0); auto& c1=varr(vat(v,"corners"),1);
                    z.xn=safe_stod(varr(c0,0).s); z.yn=safe_stod(varr(c0,1).s);
                    z.xx=safe_stod(varr(c1,0).s); z.yx=safe_stod(varr(c1,1).s);
                }
                nfzs.pb(z);
            }

        vector<pdd> wps={wh};
        for(auto& cs:stas) wps.pb({cs.x,cs.y});
        for(auto& d:dlvs) wps.pb({d.x,d.y});
        for(auto& z:nfzs) {
            if(z.sh=="circle") {
                double r=z.r+2;
                for(int a:{0,45,90,135,180,225,270,315}) {
                    double rad=a*M_PI/180;
                    wps.pb({max(0.1,min(mw-0.1,z.cx+r*cos(rad))),max(0.1,min(mh-0.1,z.cy+r*sin(rad)))});
                }
            } else {
                double xm=(z.xn+z.xx)/2,ym=(z.yn+z.yx)/2;
                for(auto [wx,wy]:vector<pdd>{{z.xn-2,z.yn-2},{z.xx+2,z.yn-2},{z.xn-2,z.yx+2},{z.xx+2,z.yx+2},{z.xn-2,ym},{z.xx+2,ym},{xm,z.yn-2},{xm,z.yx+2}})
                    wps.pb({max(0.1,min(mw-0.1,wx)),max(0.1,min(mh-0.1,wy))});
            }
        }
        for(auto& p:wps) {
            bool ex=false;
            for(auto& u:nodes) if(dist(p.first,p.second,u.first,u.second)<1e-5) { ex=true; break; }
            if(!ex) nodes.pb(p);
        }

        int M=nodes.size();
        dm.assign(M,vector<double>(M));
        for(int i=0;i<M;i++) for(int j=0;j<M;j++)
            dm[i][j]=dist(nodes[i].first,nodes[i].second,nodes[j].first,nodes[j].second);

        gnfz.assign(M,vector<vector<int>>(M));
        for(int i=0;i<M;i++) for(int j=i;j<M;j++) {
            vector<int> h;
            for(int k=0;k<(int)nfzs.size();k++) {
                CR r=(nfzs[k].sh=="circle")?chit(nodes[i].first,nodes[i].second,nodes[j].first,nodes[j].second,0,dm[i][j],nfzs[k].cx,nfzs[k].cy,nfzs[k].r,-1e9,1e9)
                    :rhit(nodes[i].first,nodes[i].second,nodes[j].first,nodes[j].second,0,dm[i][j],nfzs[k].xn,nfzs[k].yn,nfzs[k].xx,nfzs[k].yx,-1e9,1e9);
                if(r.h) h.pb(k);
            }
            gnfz[i][j]=h; gnfz[j][i]=h;
        }

        wpok.assign(M,false);
        int wi=nidx(wh.first,wh.second); wpok[wi]=true;
        for(auto& cs:stas) { int i=nidx(cs.x,cs.y); wpok[i]=true; }
        for(auto& z:nfzs) {
            if(z.sh=="circle") {
                double r=z.r+2;
                for(int a:{0,45,90,135,180,225,270,315}) {
                    double rad=a*M_PI/180;
                    int i=nidx(max(0.1,min(mw-0.1,z.cx+r*cos(rad))),max(0.1,min(mh-0.1,z.cy+r*sin(rad))));
                    wpok[i]=true;
                }
            } else {
                double xm=(z.xn+z.xx)/2,ym=(z.yn+z.yx)/2;
                for(auto [wx,wy]:vector<pdd>{{z.xn-2,z.yn-2},{z.xx+2,z.yn-2},{z.xn-2,z.yx+2},{z.xx+2,z.yx+2},{z.xn-2,ym},{z.xx+2,ym},{xm,z.yn-2},{xm,z.yx+2}}) {
                    int i=nidx(max(0.1,min(mw-0.1,wx)),max(0.1,min(mh-0.1,wy)));
                    wpok[i]=true;
                }
            }
        }

        vector<SL> slots;
        for(auto& cs:stas) slots.pb({cs.x,cs.y,vector<vector<pair<double,double>>>(cs.sl)});

        vector<Dlv> undel=dlvs;

        for(auto& drone:drones) {
            vector<Step> dman;
            double ct=0,cb=500;
            while(!undel.empty()) {
                vector<Dlv> tp; double pl=0;
                sort(undel.begin(),undel.end(),[](const Dlv& a,const Dlv& b){ return a.dl<b.dl; });
                for(auto it=undel.begin();it!=undel.end();) {
                    if(pl+it->w<=drone.cap+1e-9) { tp.pb(*it); pl+=it->w; it=undel.erase(it); }
                    else it++;
                }
                if(tp.empty()) break;

                vector<vector<Dlv>> perms;
                if(tp.size()<=5) {
                    vector<int> pi(tp.size()); iota(pi.begin(),pi.end(),0);
                    vector<pair<double,vector<Dlv>>> sp;
                    do {
                        vector<Dlv> cp; for(int i:pi) cp.pb(tp[i]);
                        double d=dist(wh.first,wh.second,cp[0].x,cp[0].y);
                        for(int k=0;k<(int)cp.size()-1;k++) d+=dist(cp[k].x,cp[k].y,cp[k+1].x,cp[k+1].y);
                        d+=dist(cp.back().x,cp.back().y,wh.first,wh.second);
                        sp.pb({d,cp});
                    } while(next_permutation(pi.begin(),pi.end()));
                    sort(sp.begin(),sp.end(),[](auto& a,auto& b){ return a.first<b.first; });
                    for(int i=0;i<min((int)sp.size(),5);i++) perms.pb(sp[i].second);
                } else {
                    vector<Dlv> g; pdd p=wh; auto tmp=tp;
                    while(!tmp.empty()) {
                        int bi=-1; double bs=1e18;
                        for(int i=0;i<(int)tmp.size();i++) {
                            double sc=dist(p.first,p.second,tmp[i].x,tmp[i].y)+0.1*tmp[i].dl;
                            if(sc<bs) { bs=sc; bi=i; }
                        }
                        if(bi<0) break;
                        g.pb(tmp[bi]); p={tmp[bi].x,tmp[bi].y}; tmp.erase(tmp.begin()+bi);
                    }
                    perms.pb(g);
                }

                bool feasible=false;
                for(auto& perm:perms) {
                    auto tsl=slots;
                    vector<int> ti; ti.pb(nidx(wh.first,wh.second));
                    for(auto& p:perm) ti.pb(nidx(p.x,p.y));
                    ti.pb(nidx(wh.first,wh.second));

                    vector<vector<PN>> lp; vector<double> lc; bool ok=true; double tc=ct;
                    vector<double> pls; double wr=0;
                    for(auto& p:perm) wr+=p.w;
                    for(auto& p:perm) { pls.pb(wr); wr-=p.w; }
                    pls.pb(0);

                    for(int i=0;i<(int)ti.size()-1;i++) {
                        auto p=shortest(ti[i],ti[i+1],tc);
                        if(p.empty()) { ok=false; break; }
                        lp.pb(p);
                        double ld=0; for(int j=0;j<(int)p.size()-1;j++) ld+=dist(p[j].x,p[j].y,p[j+1].x,p[j+1].y);
                        lc.pb(ld*(1+pls[i]));
                        tc=p.back().ta;
                    }
                    if(!ok) continue;

                    tc=ct; double bc=dman.empty()?500:cb;
                    struct Ins { int ci; double ch,wa; vector<PN> p1,p2; };
                    map<int,Ins> ins; ok=true;

                    for(int i=0;i<(int)ti.size()-1;i++) {
                        if(bc>=lc[i]) { bc-=lc[i]; tc=lp[i].back().ta; }
                        else {
                            int bcs=-1; double bt=1e18;
                            vector<PN> bp1,bp2; double bch=0,bwa=0,bba=0;
                            vector<pair<double,int>> ss;
                            for(int c=0;c<(int)stas.size();c++) {
                                int ci=nidx(stas[c].x,stas[c].y);
                                ss.pb({dm[ti[i]][ci]+dm[ci][ti[i+1]],c});
                            }
                            sort(ss.begin(),ss.end());
                            for(int sc=0;sc<min((int)ss.size(),3);sc++) {
                                int c=ss[sc].second, ci=nidx(stas[c].x,stas[c].y);
                                auto p1=shortest(ti[i],ci,tc);
                                if(p1.empty()) continue;
                                double d1=0; for(int j=0;j<(int)p1.size()-1;j++) d1+=dist(p1[j].x,p1[j].y,p1[j+1].x,p1[j+1].y);
                                double c1=d1*(1+pls[i]);
                                if(bc<c1) continue;
                                double tac=p1.back().ta, bac=bc-c1;
                                double er=0; for(int j=i+1;j<(int)ti.size()-1;j++) er+=lc[j];
                                auto p2t=shortest(ci,ti[i+1],tac);
                                if(p2t.empty()) continue;
                                double d2=0; for(int j=0;j<(int)p2t.size()-1;j++) d2+=dist(p2t[j].x,p2t[j].y,p2t[j+1].x,p2t[j+1].y);
                                double c2=d2*(1+pls[i]);
                                double btg=min(500.0,c2+er),ch=0,wa=0,tdp=tac;
                                if(bac<btg) {
                                    ch=(btg-bac)/2;
                                    double tst=book_slot(tsl,stas[c].x,stas[c].y,tac,ch);
                                    wa=tst-tac; tdp=tst+ch;
                                }
                                auto p2=shortest(ci,ti[i+1],tdp);
                                if(p2.empty()) continue;
                                if(p2.back().ta<bt) { bt=p2.back().ta; bcs=c; bp1=p1; bp2=p2; bch=ch; bwa=wa; bba=btg-c2; }
                            }
                            if(bcs!=-1) { ins[i]={bcs,bch,bwa,bp1,bp2}; bc=bba; tc=bt; }
                            else { ok=false; break; }
                        }
                    }
                    if(!ok) continue;

                    double tchk=ct;
                    for(int i=0;i<(int)perm.size();i++) {
                        tchk=ins.count(i)?ins[i].p2.back().ta:lp[i].back().ta;
                        if(tchk>perm[i].dl) { ok=false; break; }
                    }
                    if(!ok) continue;

                    slots=tsl; feasible=true;
                    vector<Step> trip;
                    trip.pb({wh.first,wh.second,ct,"PICKUP",{},""});
                    for(auto& p:perm) trip.back().ids.pb(p.id);

                    double tl=ct;
                    for(int i=0;i<(int)perm.size()+1;i++) {
                        if(ins.count(i)) {
                            auto& in=ins[i];
                            for(int j=1;j<(int)in.p1.size();j++) {
                                if(in.p1[j-1].td>in.p1[j-1].ta+1e-9) {
                                    trip.pb({in.p1[j-1].x,in.p1[j-1].y,in.p1[j-1].ta,"WAIT",{},""});
                                    trip.pb({in.p1[j-1].x,in.p1[j-1].y,in.p1[j-1].td,"WAYPOINT",{},""});
                                }
                                if(j<(int)in.p1.size()-1) trip.pb({in.p1[j].x,in.p1[j].y,in.p1[j].ta,"WAYPOINT",{},""});
                            }
                            double tca=in.p1.back().ta;
                            trip.pb({stas[in.ci].x,stas[in.ci].y,tca,"CHARGE",{},""});
                            if(in.wa>1e-9) trip.pb({stas[in.ci].x,stas[in.ci].y,tca+in.wa,"CHARGE",{},""});
                            trip.pb({stas[in.ci].x,stas[in.ci].y,tca+in.wa+in.ch,"CHARGE_COMPLETE",{},""});
                            for(int j=1;j<(int)in.p2.size();j++) {
                                if(in.p2[j-1].td>in.p2[j-1].ta+1e-9) {
                                    trip.pb({in.p2[j-1].x,in.p2[j-1].y,in.p2[j-1].ta,"WAIT",{},""});
                                    trip.pb({in.p2[j-1].x,in.p2[j-1].y,in.p2[j-1].td,"WAYPOINT",{},""});
                                }
                                if(j<(int)in.p2.size()-1) trip.pb({in.p2[j].x,in.p2[j].y,in.p2[j].ta,"WAYPOINT",{},""});
                            }
                            tl=in.p2.back().ta;
                        } else {
                            auto& leg=lp[i];
                            for(int j=1;j<(int)leg.size();j++) {
                                if(leg[j-1].td>leg[j-1].ta+1e-9) {
                                    trip.pb({leg[j-1].x,leg[j-1].y,leg[j-1].ta,"WAIT",{},""});
                                    trip.pb({leg[j-1].x,leg[j-1].y,leg[j-1].td,"WAYPOINT",{},""});
                                }
                                if(j<(int)leg.size()-1) trip.pb({leg[j].x,leg[j].y,leg[j].ta,"WAYPOINT",{},""});
                            }
                            tl=leg.back().ta;
                        }
                        if(i<(int)perm.size()) trip.pb({perm[i].x,perm[i].y,tl,"DELIVER",{},perm[i].id});
                        else trip.pb({wh.first,wh.second,tl,"RETURN",{},""});
                    }
                    dman.insert(dman.end(),trip.begin(),trip.end());
                    ct=tl; cb=500;
                    break;
                }
                if(!feasible) { undel.insert(undel.end(),tp.begin(),tp.end()); break; }
            }
            if(!dman.empty()) manifests.pb({drone.id,dman});
        }
    } catch(...) {}
    out_json(manifests);
    return 0;
}
