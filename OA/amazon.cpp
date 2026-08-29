string getSmallestBaseSegment(int segmentSize, string missingData) {
    vector<int> freq(26, 0);

    for (char c : missingData) {
        freq[c - 'a']++;
    }

    int distinct = 0;
    for (int x : freq) {
        if (x > 0) distinct++;
    }

    if (distinct > segmentSize) {
        return "-1";
    }

    int n = missingData.size();


    int low = 1, high = n;

    while (low < high) {
        int mid = low + (high - low) / 2;

        int required = 0;

        for (int x : freq) {
            if (x > 0) {
                required += (x + mid - 1) / mid;

                if (required > segmentSize)
                    break;
            }
        }

        if (required <= segmentSize)
            high = mid;
        else
            low = mid + 1;
    }

    int replications = low;


    vector<int> cnt(26, 0);
    int used = 0;

    for (int i = 0; i < 26; i++) {
        if (freq[i] > 0) {
            cnt[i] = (freq[i] + replications - 1) / replications;
            used += cnt[i];
        }
    }

    cnt[0] += segmentSize - used;

    string result;

    for (int i = 0; i < 26; i++) {
        result.append(cnt[i], char('a' + i));
    }

    return result;
}