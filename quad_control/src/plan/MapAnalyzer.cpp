#include "MapAnalyzer.h"

#include <cmath>
#include <ros/console.h>

/* If map[i] > TRESHOLD_VAL, it is assumed that there's an obstacle in that
 * cell. Map values goes from 0 to 100: for map retrieved from the octomap,
 * map only contains value 0 or 100, and nothing in between.
 */
#define TRESHOLD_VAL    50

MapAnalyzer::MapAnalyzer(){
    _map = 0;
    _mapReady = false;
    _scanned = false;
    _w = _h = 0;
    _visited = 0;
}

MapAnalyzer::~MapAnalyzer(){
    for (auto chunk : this->_chunks)
        _deleteTree(chunk);
    delete this->_visited;
}

void MapAnalyzer::_deleteTree(Chunk *root){
    if(root->left)  _deleteTree(root->left);
    if(root->right) _deleteTree(root->right);
    if(root->down)  _deleteTree(root->down);
    delete root;
}


void MapAnalyzer::_fillTree(Chunk *root, int index){
    if(!this->_map || !this->_visited)
        return;

    this->_visited[index] = true;
    if(this->_map[index] < TRESHOLD_VAL)
        return;

    // Left node
    if(root->y > 0){
        int k = index - 1;
        if (!this->_visited[k] && this->_map[k] > TRESHOLD_VAL){
            root->left = new Chunk;
            root->left->x = root->x;
            root->left->y = root->y-1;
            _fillTree(root->left, k);
        }
    }

    // Right node
    if(root->y < this->_w - 1){
        int k = index + 1;
        if (!this->_visited[k] && this->_map[k] > TRESHOLD_VAL){
            root->right = new Chunk;
            root->right->x = root->x;
            root->right->y = root->y+1;
            _fillTree(root->right, k);
        }
    }

    // Bottom node
    if(root->x > 0){
        int k = index - this->_w;
        if (!this->_visited[k] && this->_map[k] > TRESHOLD_VAL){
            root->down = new Chunk;
            root->down->x = root->x-1;
            root->down->y = root->y;
            _fillTree(root->down, k);
        }
    }
}


void MapAnalyzer::setMap(nav_msgs::OccupancyGrid &grid){
    this->setMap(&grid.data[0], grid.info.width, grid.info.height);
}


void MapAnalyzer::setMap(int8_t *map, int w, int h){
    this->_map = map;
    int size = w*h;

    // Clear chunks
    for (auto chunk : this->_chunks)
        _deleteTree(chunk);
    this->_chunks.clear();

    // Clear visited
    delete this->_visited;
    this->_visited = new bool[size];
    for (int i=0; i<size; ++i)
        this->_visited[i] = false;

    this->_w = w;
    this->_h = h;

    this->_mapReady = true;
}


void MapAnalyzer::scan(){
    ROS_INFO("Scanning map...");

    // Scan the map
    for (int r=_h-1; r>=0; --r){
        for (int c=0; c < _w; ++c){
            int i = r * _w + c;

            if (this->_map[i] < TRESHOLD_VAL || this->_visited[i]){
                this->_visited[i] = true;
                continue;
            }

            // Fill tree
            Chunk *obst = new Chunk;
            // Coords are inverted wrt the usual way, since the frame is NED
            obst->y = i % _w;
            obst->x = i / _w;
            this->_fillTree(obst, i);

            this->_chunks.push_back(obst);
        }
    }

    _scanned = true;
    ROS_INFO("Map scanned.");
}


Chunk* MapAnalyzer::_computeChunkDist(Chunk *chunk, int rx, int ry){
    if (chunk == 0)
        return 0;

    Chunk *best = chunk;
    Chunk *temp = 0;
    chunk->dist2 = pow(chunk->x - rx, 2) + pow(chunk->y - ry, 2);

    if(chunk->left){
        temp = _computeChunkDist(chunk->left, rx, ry);
        if(temp->dist2 < best->dist2)
            best = temp;
    }

    if(chunk->right){
        temp = _computeChunkDist(chunk->right, rx, ry);
        if(temp->dist2 < best->dist2)
            best = temp;
    }

    if(chunk->down){
        temp = _computeChunkDist(chunk->down, rx, ry);
        if(temp->dist2 < best->dist2)
            best = temp;
    }

    return best;
}


vector<Chunk*> MapAnalyzer::getObjAtMinDist(int rx, int ry){
    vector<Chunk*> mins;

    // Compute distances
    for (auto chunk : this->_chunks)
        mins.push_back(_computeChunkDist(chunk, rx, ry));

    return mins;
}