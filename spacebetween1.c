int solution(string &S){
    int cnt=0;
    int i;
    for(i=0;i<S.length();i++){
        if(S[i]=='1'){
            break;
        }
    }

    S = S.substr(i);
    for(int j=S.length()-1;j>=0;j--){
        if(S[j]=='1') cnt+=2;
        else cnt++;
    }

    cnt--;
    return cnt;
}
