#include<iostream>
#include<cstdint> 
#include<bitset> 
#include<string> 
#include<vector>
#include<algorithm> 
#include <sstream>
#include <iomanip> 
#include <chrono> 

using namespace std; 

//Function to convert message to binary 
string messageToBinary(string message){

    string binaryString; 
    for(char c: message){
        bitset<8> charBits (static_cast<unsigned char>(c));
        //cout<<"\nCharacter bits for "<<c<<" = "<<charBits;  
        binaryString += charBits.to_string();
    }
    return binaryString; 

}



//Function to append one at the end of the message 
string appendOne(string binaryString){
    binaryString += "1"; 
    return binaryString; 
}

//Function to pad zeros if length of message is less than 448 bit 
string padZeros(string binaryString){
    int messageLength = binaryString.size(); 

    //for a message which is less than 448 bits in length 
    if(messageLength <= 448){
        int numZeros = 448 - messageLength; 
        //cout<<"\nNumber of zeros to be padded = "<<numZeros; 
        string paddedZeros(numZeros,'0');
        //cout<<"\nLength of padded zero string = "<<paddedZeros.size(); 
        binaryString += paddedZeros; 

    }
    //for multiple blocks 
    else{
        int numZeros = (449 - (messageLength + 1)) % 512;
        if (numZeros < 0){
            numZeros += 512;
        }
        
        //cout<<"\nNumber of zeros to be padded = "<<numZeros; 
        string paddedZeros(numZeros,'0'); 
        //cout<<"\nLength of padded zero string = "<<paddedZeros.size(); 
        binaryString += paddedZeros; 
    }
    return binaryString;
}



//Function to store the length of the message as 64 bit 
string preprocessedString(string message){

    //string message_copy = message; 
    string preprocessedMessage;  
    string binaryString = messageToBinary(message); 
    uint64_t stringLength = binaryString.size(); 
    //cout<<"\nLength of binary string = "<<stringLength;
    bitset<64> lengthBits(stringLength); 
    string stringLengthAsString = lengthBits.to_string(); 
    //cout<<"\nString length as bits = "<<lengthBits;
    //cout<<"\nString length as string = "<<stringLengthAsString; 
    //cout<<"\nLength of string length as string = "<<stringLengthAsString.size();

    binaryString = appendOne(binaryString); 
    //cout<<"\nBinary string with 1 appended = "<<binaryString; 
    binaryString = padZeros(binaryString); 
    
    preprocessedMessage = binaryString + stringLengthAsString;  
    
    return preprocessedMessage; 
}



bitset<32> ROTR(bitset<32> word, int n){
    return ((word >> n) | (word << 32 - n));
}

bitset<32> sigma0(bitset<32> word){

    return ROTR(word, 7) ^ ROTR(word, 18) ^ (word >> 3);  

}

bitset<32> sigma1(bitset<32> word){

    return ROTR(word, 17) ^ ROTR(word, 19) ^ (word >> 10);  

}


//Word expansion is producing the correct output 

vector<bitset<32>> wordExpansionSingleBlock(string preprocessedMessage){
    
    
    vector<bitset<32>>words(64); 

    for(int i = 0; i < 16; i++){
        words[i] = bitset<32>(preprocessedMessage.substr(i*32,32));
        //std::stringstream ssw;
        //ssw << std::hex << words[i].to_ullong(); // Convert to hexadecimal
        //std::cout <<"\nWord "<<i+1<<" = "<<ssw.str();
        //cout<<"\nWord "<<i+1<<" = "<<words[i];
    }

    uint32_t temp; 

    for(int i = 16; i < 64; i++){
        temp = words[i - 16].to_ulong() + sigma0(words[i - 15]).to_ulong() + words[i - 7].to_ulong() + sigma1(words[i - 2]).to_ulong(); 
        words[i] = bitset<32>(temp);
    }
    
    // for(int i = 16; i < 64; i++){
    //     std::stringstream ss;
    //     ss << std::hex << words[i].to_ullong(); // Convert to hexadecimal
    //     std::cout <<"\nWord "<<i+1<<" = "<<ss.str();
    //     //cout<<"\nWord "<<i+1<<" = "<<words[i];
    // }

    return words; 
}

//Correct output produced for single block of data 
pair<vector<uint32_t>, vector<bitset<32>> > computeSHA256SingleBlock(vector<bitset<32>> words, vector<uint32_t> inputHashes){

    
    bitset<32>S1; 
    bitset<32>S0;
    bitset<32>ch; 
    bitset<32>maj;
    uint32_t temp1, temp2; 

    //round constants k0 to k63 
    vector<uint32_t> k = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    //initialized a,b,c,d,e,f,g,h 
    bitset<32> a_hash = bitset<32>(inputHashes[0]);
    bitset<32> b_hash = bitset<32>(inputHashes[1]); 
    bitset<32> c_hash = bitset<32>(inputHashes[2]); 
    bitset<32> d_hash = bitset<32>(inputHashes[3]);
    bitset<32> e_hash = bitset<32>(inputHashes[4]); 
    bitset<32> f_hash = bitset<32>(inputHashes[5]);
    bitset<32> g_hash = bitset<32>(inputHashes[6]); 
    bitset<32> h_hash = bitset<32>(inputHashes[7]);

    for(int i = 0; i < 64; i++){

        S1 = ROTR(e_hash, 6) ^ ROTR(e_hash, 11) ^ ROTR(e_hash, 25); 
        ch = (e_hash & f_hash) ^ ((~e_hash) & g_hash); 
        temp1 = h_hash.to_ulong() + S1.to_ulong() + ch.to_ulong() + k[i] + words[i].to_ulong(); 
        S0 = ROTR(a_hash, 2) ^ ROTR(a_hash, 13) ^ ROTR(a_hash, 22); 
        maj = (a_hash & b_hash) ^ (b_hash & c_hash) ^ (c_hash & a_hash); 
        temp2 = S0.to_ulong() + maj.to_ulong(); 

        // std::stringstream ss_s1;
        // ss_s1 << std::hex << S1.to_ulong(); // Convert to hexadecimal
        // //std::cout <<"\nRound "<<i<<": S1 = "<<ss_s1.str();

        // std::stringstream ss_ch;
        // ss_ch << std::hex << ch.to_ulong(); // Convert to hexadecimal
        // //std::cout <<"\nRound "<<i<<": ch = "<<ss_ch.str();

        // std::stringstream ss_s0; 
        // ss_s0 << std::hex << S0.to_ulong(); 

        // std::stringstream ss_maj; 
        // ss_maj << std::hex << maj.to_ulong(); 

        //std::cout <<"\nRound "<<i<<": S1 = "<<ss_s1.str()<<", ch = "<<ss_ch.str()<<", S0 = "<<ss_s0.str()<<", maj = "<<ss_maj.str();

        h_hash = g_hash; 
        g_hash = f_hash; 
        f_hash = e_hash; 
        e_hash = bitset<32>(d_hash.to_ulong() + temp1);
        d_hash = c_hash; 
        c_hash = b_hash; 
        b_hash = a_hash; 
        a_hash = bitset<32>(temp1 + temp2);  

    }

    inputHashes[0] += a_hash.to_ulong();
    inputHashes[1] += b_hash.to_ulong();
    inputHashes[2] += c_hash.to_ulong();
    inputHashes[3] += d_hash.to_ulong();
    inputHashes[4] += e_hash.to_ulong();
    inputHashes[5] += f_hash.to_ulong();
    inputHashes[6] += g_hash.to_ulong(); 
    inputHashes[7] += h_hash.to_ulong();

    vector<bitset<32>> inputHashVector = {a_hash,b_hash,c_hash,d_hash,e_hash,f_hash,g_hash,h_hash}; 
    
    return {inputHashes, inputHashVector}; 

}

//Word expansion for multiple blocks 
vector<vector<bitset<32>>> wordExpansionMultipleBlocks(string preprocessedMessage){
    
    int numBlocks = preprocessedMessage.size()/512; 
    //int preprocessedMessageLength = preprocessedMessage.size();
    vector<bitset<32>>words(64);
    vector<vector<bitset<32>>> multiWordBlocks;
    
    for(int b = 0; b < numBlocks; b++){

        words = wordExpansionSingleBlock(preprocessedMessage.substr(b*512,512)); 

        multiWordBlocks.push_back(words);

    }

    return multiWordBlocks; 
}


//Correct output produced for single block of data 
vector<uint32_t> computeSHA256MultipleBlocks(vector<vector<bitset<32>>> multiWordBlocks, vector<uint32_t> initialHashes){
    pair<vector<uint32_t>, vector<bitset<32>>> finalHashAndHashValues; 
    for(int b = 0; b < multiWordBlocks.size(); b++){

        finalHashAndHashValues = computeSHA256SingleBlock(multiWordBlocks[b], initialHashes);

        initialHashes = finalHashAndHashValues.first; 

    }
    
    return initialHashes; 

}

std::string hashToHexUTF8(const std::vector<uint32_t>& hash) {
    std::ostringstream oss;
    for (uint32_t word : hash) {
        // Output each 32-bit word as 8 hex digits (big-endian)
        oss << std::hex << std::setfill('0')
            << std::setw(2) << ((word >> 24) & 0xFF)
            << std::setw(2) << ((word >> 16) & 0xFF)
            << std::setw(2) << ((word >> 8) & 0xFF)
            << std::setw(2) << (word & 0xFF);
    }
    return oss.str();
}

int main(){

    string message = "Hello Earthlings! It's me Vaibhav speaking to you from Mindgrove. I am happy to be back. WOOHOOO!!!!! System Software Engineering is fun! I am excited about this new job! I successfully got SHA256 to work for multiple blocks! IITM RP is full of intelligent and hard working people.";
    //string message = "System Software is Fun!"; 
    string preprocessedMessage = preprocessedString(message); 
    //cout<<"\nPreprocessed Message Binary = "<<preprocessedMessage; 
    cout<<"\nLength of preprocessed message = "<<preprocessedMessage.size(); 

    //initial hashes are constants 
    vector<uint32_t> initialHashes =  {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    if(preprocessedMessage.size() == 512){
        cout<<"\nPreprocessed Message is a single data block";

        auto start = std::chrono::high_resolution_clock::now();
        vector<bitset<32>>words = wordExpansionSingleBlock(preprocessedMessage);         
        pair<vector<uint32_t>, vector<bitset<32>>> finalHashAndHashValues = computeSHA256SingleBlock(words, initialHashes); 
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start; 
        std::string hexHash = hashToHexUTF8(finalHashAndHashValues.first);
        std::cout << "\nSHA-256 hash in hex: " << hexHash << std::endl;
        std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;
    }
    else{
        cout<<"\nPreprocessed Message is a multiple data block";
        int numBlocks = preprocessedMessage.size()/512; 
        cout<<"\nPreprocessed Message contains "<<numBlocks<<" blocks"; 
        auto start = std::chrono::high_resolution_clock::now();
        vector<vector<bitset<32>>>wordBlocks = wordExpansionMultipleBlocks(preprocessedMessage);
        vector<uint32_t>hash = computeSHA256MultipleBlocks(wordBlocks, initialHashes); 
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start; 


        std::string hexHash = hashToHexUTF8(hash);
        std::cout << "\nSHA-256 hash in hex: " << hexHash << std::endl;
        std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    }

    cout<<"\n"; 
    return 0; 

}